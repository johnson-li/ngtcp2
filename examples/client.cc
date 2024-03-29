/*
 * ngtcp2
 *
 * Copyright (c) 2017 ngtcp2 contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <cstdlib>
#include <cassert>
#include <cerrno>
#include <iostream>
#include <algorithm>
#include <memory>
#include <fstream>

#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/mman.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "stdlib.h"   
#include "stdio.h" 

#include "client.h"
#include "network.h"
#include "debug.h"
#include "util.h"
#include "crypto.h"
#include "shared.h"

using namespace ngtcp2;

namespace {
auto randgen = util::make_mt19937();
} // namespace

namespace {
Config config{};
} // namespace

namespace {
constexpr size_t MAX_BYTES_IN_FLIGHT = 1460 * 10;
} // namespace

namespace {
int create_sock(Address &remote_addr, const char *addr, const char *port) {
  addrinfo hints{};
  addrinfo *res, *rp;
  int rv;
  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  rv = getaddrinfo(addr, port, &hints, &res);
  if (rv != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    return -1;
  }

  auto res_d = defer(freeaddrinfo, res);

  int fd = -1;

  struct sockaddr_in6 client_addr;
  client_addr.sin6_family = AF_INET6; 
  inet_pton(AF_INET6, "::1", &(client_addr.sin6_addr.s6_addr)); 
  client_addr.sin6_port = htons(12345); 
  
  for (rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    std::cout << "tjx: " << rp->ai_family << " ipv6: " << AF_INET6 << std::endl;
    if (fd == -1) {
      continue;
    }
    

    if (connect(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
      goto next;
    }

    break;


  next:
    close(fd);
  }

  if (!rp) {
    std::cerr << "Could not connect" << std::endl;
    return -1;
  }

  auto val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val,
                 static_cast<socklen_t>(sizeof(val))) == -1) {
    return -1;
  }
  
  struct sockaddr_in null_addr;
  null_addr.sin_family = AF_UNSPEC; 

  connect(fd, (struct sockaddr*)&null_addr, sizeof(null_addr));
  ::bind(fd, (struct sockaddr*)&client_addr, sizeof(client_addr));
  remote_addr.len = rp->ai_addrlen;
  memcpy(&remote_addr.su, rp->ai_addr, rp->ai_addrlen);

  return fd;
}

} // namespace

Buffer::Buffer(const uint8_t *data, size_t datalen)
    : buf{data, data + datalen},
      begin(buf.data()),
      head(begin),
      tail(begin + datalen) {}
Buffer::Buffer(uint8_t *begin, uint8_t *end)
    : begin(begin), head(begin), tail(end) {}
Buffer::Buffer(size_t datalen)
    : buf(datalen), begin(buf.data()), head(begin), tail(begin) {}
Buffer::Buffer() : begin(buf.data()), head(begin), tail(begin) {}

Stream::Stream(uint64_t stream_id)
    : stream_id(stream_id),
      streambuf_idx(0),
      tx_stream_offset(0),
      should_send_fin(false) {}

Stream::~Stream() {}

void Stream::buffer_file() {
  streambuf.emplace_back(config.data, config.data + config.datalen);
  should_send_fin = true;
}

namespace {
int bio_write(BIO *b, const char *buf, int len) {
  int rv;

  BIO_clear_retry_flags(b);

  auto c = static_cast<Client *>(BIO_get_data(b));
  
  rv = c->write_client_handshake(reinterpret_cast<const uint8_t *>(buf), len);
  if (rv != 0) {
    return -1;
  }

  return len;
}
} // namespace

namespace {
int bio_read(BIO *b, char *buf, int len) {
  BIO_clear_retry_flags(b);

  auto c = static_cast<Client *>(BIO_get_data(b));

  len = c->read_server_handshake(reinterpret_cast<uint8_t *>(buf), len);
  if (len == 0) {
    BIO_set_retry_read(b);
    return -1;
  }

  return len;
}
} // namespace

namespace {
int bio_puts(BIO *b, const char *str) { return bio_write(b, str, strlen(str)); }
} // namespace

namespace {
int bio_gets(BIO *b, char *buf, int len) { return -1; }
} // namespace

namespace {
long bio_ctrl(BIO *b, int cmd, long num, void *ptr) {
  switch (cmd) {
  case BIO_CTRL_FLUSH:
    return 1;
  }

  return 0;
}
} // namespace

namespace {
int bio_create(BIO *b) {
  BIO_set_init(b, 1);
  return 1;
}
} // namespace

namespace {
int bio_destroy(BIO *b) {
  if (b == nullptr) {
    return 0;
  }

  return 1;
}
} // namespace

namespace {
BIO_METHOD *create_bio_method() {
  static auto meth = BIO_meth_new(BIO_TYPE_FD, "bio");
  BIO_meth_set_write(meth, bio_write);
  BIO_meth_set_read(meth, bio_read);
  BIO_meth_set_puts(meth, bio_puts);
  BIO_meth_set_gets(meth, bio_gets);
  BIO_meth_set_ctrl(meth, bio_ctrl);
  BIO_meth_set_create(meth, bio_create);
  BIO_meth_set_destroy(meth, bio_destroy);
  return meth;
}
} // namespace

namespace {
void writecb(struct ev_loop *loop, ev_io *w, int revents) {
  ev_io_stop(loop, w);

  auto c = static_cast<Client *>(w->data);
  std::cout << "tjx: write data: " << w->data << std::endl;

  auto rv = c->on_write();
  if (rv == NETWORK_ERR_SEND_FATAL) {
    c->disconnect();
    return;
  }
}
} // namespace

namespace {
void readcb(struct ev_loop *loop, ev_io *w, int revents) {
  auto c = static_cast<Client *>(w->data);

  if (c->on_read() != 0) {
    c->disconnect();
    return;
  }
  auto rv = c->on_write();
  if (rv == NETWORK_ERR_SEND_FATAL) {
    c->disconnect();
  }
}
} // namespace

namespace {
void stdin_readcb(struct ev_loop *loop, ev_io *w, int revents) {
  auto c = static_cast<Client *>(w->data);

  if (c->send_interactive_input()) {
    c->disconnect();
  }
}
} // namespace

namespace {
void timeoutcb(struct ev_loop *loop, ev_timer *w, int revents) {
  auto c = static_cast<Client *>(w->data);

  if (!config.quiet) {
    debug::print_timestamp();
    std::cerr << "Timeout" << std::endl;
  }

  c->disconnect();
}
} // namespace

namespace {
void retransmitcb(struct ev_loop *loop, ev_timer *w, int revents) {
  auto c = static_cast<Client *>(w->data);

  if (c->on_write() != 0) {
    c->disconnect();
  }
}
} // namespace

namespace {
void siginthandler(struct ev_loop *loop, ev_signal *w, int revents) {
  ev_break(loop, EVBREAK_ALL);
}
} // namespace

Client::Client(struct ev_loop *loop, SSL_CTX *ssl_ctx)
    : remote_addr_{},
      max_pktlen_(0),
      loop_(loop),
      ssl_ctx_(ssl_ctx),
      ssl_(nullptr),
      fd_(-1),
      datafd_(-1),
      chandshake_idx_(0),
      nsread_(0),
      conn_(nullptr),
      crypto_ctx_{},
      sendbuf_{NGTCP2_MAX_PKTLEN_IPV6},
      last_stream_id_(0),
      nstreams_done_(0),
      resumption_(false) {
  ev_io_init(&wev_, writecb, 0, EV_WRITE);
  ev_io_init(&rev_, readcb, 0, EV_READ);
  ev_io_init(&stdinrev_, stdin_readcb, 0, EV_READ);
  wev_.data = this;
  rev_.data = this;
  stdinrev_.data = this;
  ev_timer_init(&timer_, timeoutcb, 0., config.timeout);
  timer_.data = this;
  ev_timer_init(&rttimer_, retransmitcb, 0., 0.);
  rttimer_.data = this;
  ev_signal_init(&sigintev_, siginthandler, SIGINT);
}

Client::~Client() {
  disconnect();
  close();
}

void Client::disconnect() { disconnect(0); }

void Client::disconnect(int liberr) {
  config.tx_loss_prob = 0;

  ev_timer_stop(loop_, &rttimer_);
  ev_timer_stop(loop_, &timer_);

  ev_io_stop(loop_, &stdinrev_);
  ev_io_stop(loop_, &rev_);

  ev_signal_stop(loop_, &sigintev_);

  handle_error(liberr);
}

void Client::close() {
  ev_io_stop(loop_, &wev_);

  if (conn_) {
    ngtcp2_conn_del(conn_);
    conn_ = nullptr;
  }

  if (ssl_) {
    SSL_free(ssl_);
    ssl_ = nullptr;
  }

  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
}

namespace {
ssize_t send_client_initial(ngtcp2_conn *conn, uint32_t flags,
                            uint64_t *ppkt_num, const uint8_t **pdest,
                            void *user_data) {
  auto c = static_cast<Client *>(user_data);

  if (c->tls_handshake(true) != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  c->handle_early_data();
  std::cout << "tjx: send initial!" << std::endl;

  if (ppkt_num) {
    *ppkt_num = std::uniform_int_distribution<uint64_t>(
        0, NGTCP2_MAX_INITIAL_PKT_NUM)(randgen);
  }
  
  auto len = c->read_client_handshake(pdest);
  
  return len;
}
} // namespace

namespace {
ssize_t send_client_handshake(ngtcp2_conn *conn, uint32_t flags,
                              const uint8_t **pdest, void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto len = c->read_client_handshake(pdest);

  return len;
}
} // namespace

namespace {
int recv_stream0_data(ngtcp2_conn *conn, const uint8_t *data, size_t datalen,
                      void *user_data) {
  auto c = static_cast<Client *>(user_data);

  c->write_server_handshake(data, datalen);

  if (ngtcp2_conn_get_handshake_completed(c->conn())) {
    return c->read_tls();
  } else if (c->tls_handshake() != 0) {
    return NGTCP2_ERR_TLS_HANDSHAKE;
  }
  // TODO Should we call c->read_tls if handshake has finished?
  // OpenSSL has also weird pending state.

  return 0;
}
} // namespace

namespace {
int recv_stream_data(ngtcp2_conn *conn, uint64_t stream_id, uint8_t fin,
                     const uint8_t *data, size_t datalen, void *user_data,
                     void *stream_user_data) {
  if (!config.quiet) {
    debug::print_stream_data(stream_id, data, datalen);
  }
  ngtcp2_conn_extend_max_stream_offset(conn, stream_id, datalen);
  ngtcp2_conn_extend_max_offset(conn, datalen);
  return 0;
}
} // namespace

namespace {
int acked_stream_data_offset(ngtcp2_conn *conn, uint64_t stream_id,
                             uint64_t offset, size_t datalen, void *user_data,
                             void *stream_user_data) {
  auto c = static_cast<Client *>(user_data);
  if (c->remove_tx_stream_data(stream_id, offset, datalen) != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }
  return 0;
}
} // namespace

namespace {
int handshake_completed(ngtcp2_conn *conn, void *user_data) {
  auto c = static_cast<Client *>(user_data);
  
  std::cout << "tjx: output_addr:" << c->remote_addr_.su.in6.sin6_addr.s6_addr << std::endl;

  if (!config.quiet) {
    debug::handshake_completed(conn, user_data);
  }

  if (c->setup_crypto_context() != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return 0;
}
} // namespace

namespace {
int recv_server_stateless_retry(ngtcp2_conn *conn, void *user_data) {
  return 0;
}
} // namespace

namespace {
int stream_close(ngtcp2_conn *conn, uint64_t stream_id, uint16_t app_error_code,
                 void *user_data, void *stream_user_data) {
  auto c = static_cast<Client *>(user_data);

  c->on_stream_close(stream_id);

  return 0;
}
} // namespace

namespace {
int extend_max_stream_id(ngtcp2_conn *conn, uint64_t max_stream_id,
                         void *user_data) {
  auto c = static_cast<Client *>(user_data);

  if (c->on_extend_max_stream_id(max_stream_id) != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return 0;
}

} // namespace

namespace {
ssize_t do_hs_encrypt(ngtcp2_conn *conn, uint8_t *dest, size_t destlen,
                      const uint8_t *plaintext, size_t plaintextlen,
                      const uint8_t *key, size_t keylen, const uint8_t *nonce,
                      size_t noncelen, const uint8_t *ad, size_t adlen,
                      void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto nwrite = c->hs_encrypt_data(dest, destlen, plaintext, plaintextlen, key,
                                   keylen, nonce, noncelen, ad, adlen);
  if (nwrite < 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return nwrite;
}
} // namespace

namespace {
ssize_t do_hs_decrypt(ngtcp2_conn *conn, uint8_t *dest, size_t destlen,
                      const uint8_t *ciphertext, size_t ciphertextlen,
                      const uint8_t *key, size_t keylen, const uint8_t *nonce,
                      size_t noncelen, const uint8_t *ad, size_t adlen,
                      void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto nwrite = c->hs_decrypt_data(dest, destlen, ciphertext, ciphertextlen,
                                   key, keylen, nonce, noncelen, ad, adlen);
  if (nwrite < 0) {
    return NGTCP2_ERR_TLS_DECRYPT;
  }

  return nwrite;
}
} // namespace

namespace {
ssize_t do_encrypt(ngtcp2_conn *conn, uint8_t *dest, size_t destlen,
                   const uint8_t *plaintext, size_t plaintextlen,
                   const uint8_t *key, size_t keylen, const uint8_t *nonce,
                   size_t noncelen, const uint8_t *ad, size_t adlen,
                   void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto nwrite = c->encrypt_data(dest, destlen, plaintext, plaintextlen, key,
                                keylen, nonce, noncelen, ad, adlen);
  if (nwrite < 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return nwrite;
}
} // namespace

namespace {
ssize_t do_decrypt(ngtcp2_conn *conn, uint8_t *dest, size_t destlen,
                   const uint8_t *ciphertext, size_t ciphertextlen,
                   const uint8_t *key, size_t keylen, const uint8_t *nonce,
                   size_t noncelen, const uint8_t *ad, size_t adlen,
                   void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto nwrite = c->decrypt_data(dest, destlen, ciphertext, ciphertextlen, key,
                                keylen, nonce, noncelen, ad, adlen);
  if (nwrite < 0) {
    return NGTCP2_ERR_TLS_DECRYPT;
  }

  return nwrite;
}
} // namespace

int Client::init(int fd, const Address &remote_addr, const char *addr,
                 int datafd, uint32_t version) {
  int rv;

  remote_addr_ = remote_addr;

  switch (remote_addr_.su.storage.ss_family) {
  case AF_INET:
    max_pktlen_ = NGTCP2_MAX_PKTLEN_IPV4;
    break;
  case AF_INET6:
    max_pktlen_ = NGTCP2_MAX_PKTLEN_IPV6;
    break;
  default:
    return -1;
  }

  fd_ = fd;
  datafd_ = datafd;
/*
  if (-1 == connect(fd_, &remote_addr_.su.sa, remote_addr_.len)) {
    std::cerr << "connect: " << strerror(errno) << std::endl;
    return -1;
  }
  std::cout << "tjx: why need connect like tcp" << std::endl;
*/

  ssl_ = SSL_new(ssl_ctx_);
  auto bio = BIO_new(create_bio_method());
  BIO_set_data(bio, this);
  SSL_set_bio(ssl_, bio, bio);
  SSL_set_app_data(ssl_, this);
  SSL_set_connect_state(ssl_);

  const uint8_t *alpn = nullptr;
  size_t alpnlen;

  switch (version) {
  case NGTCP2_PROTO_VER_D8:
    alpn = reinterpret_cast<const uint8_t *>(NGTCP2_ALPN_D8);
    alpnlen = str_size(NGTCP2_ALPN_D8);
    break;
  }
  if (alpn) {
    SSL_set_alpn_protos(ssl_, alpn, alpnlen);
  }

  if (util::numeric_host(addr)) {
    // If remote host is numeric address, just send "localhost" as SNI
    // for now.
    SSL_set_tlsext_host_name(ssl_, "localhost");
  } else {
    SSL_set_tlsext_host_name(ssl_, addr);
  }
  std::cout << "tjx: addr: " << addr << std::endl;

  if (config.session_file) {
    auto f = BIO_new_file(config.session_file, "r");
    if (f == nullptr) {
      std::cerr << "Could not read TLS session file " << config.session_file
                << std::endl;
    } else {
      auto session = PEM_read_bio_SSL_SESSION(f, nullptr, 0, nullptr);
      BIO_free(f);
      if (session == nullptr) {
        std::cerr << "Could not read TLS session file " << config.session_file
                  << std::endl;
      } else {
        if (!SSL_set_session(ssl_, session)) {
          std::cerr << "Could not set session" << std::endl;
        } else {
          resumption_ = true;
        }
        SSL_SESSION_free(session);
      }
    }
  }

  auto callbacks = ngtcp2_conn_callbacks{
      send_client_initial,
      send_client_handshake,
      nullptr,
      nullptr,
      recv_stream0_data,
      config.quiet ? nullptr : debug::send_pkt,
      config.quiet ? nullptr : debug::send_frame,
      config.quiet ? nullptr : debug::recv_pkt,
      config.quiet ? nullptr : debug::recv_frame,
      handshake_completed,
      config.quiet ? nullptr : debug::recv_version_negotiation,
      do_hs_encrypt,
      do_hs_decrypt,
      do_encrypt,
      do_decrypt,
      recv_stream_data,
      acked_stream_data_offset,
      stream_close,
      config.quiet ? nullptr : debug::recv_stateless_reset,
      recv_server_stateless_retry,
      extend_max_stream_id,
  };

  auto conn_id = std::uniform_int_distribution<uint64_t>(
      0, std::numeric_limits<uint64_t>::max())(randgen);

  ngtcp2_settings settings;
  settings.max_stream_data = 256_k;
  settings.max_data = 1_m;
  settings.max_stream_id_bidi = 1;
  settings.max_stream_id_uni = 3;
  settings.idle_timeout = config.timeout;
  settings.omit_connection_id = 0;
  settings.max_packet_size = NGTCP2_MAX_PKT_SIZE;
  settings.server_unicast_ip[0] = 0;
  settings.server_unicast_ip[1] = 0;
  settings.server_unicast_ip[2] = 0;
  settings.server_unicast_ip[3] = 0;
  //settings.server_unicast_ttl = 0;
  settings.ack_delay_exponent = NGTCP2_DEFAULT_ACK_DELAY_EXPONENT;

  rv = ngtcp2_conn_client_new(&conn_, conn_id, version, &callbacks, &settings,
                              this);

  std::cout << "tjx: new connection!" << std::endl;
  if (rv != 0) {
    std::cerr << "ngtcp2_conn_client_new: " << ngtcp2_strerror(rv) << std::endl;
    return -1;
  }

  std::array<uint8_t, 32> handshake_secret, secret;
  conn_id = ngtcp2_conn_negotiated_conn_id(conn_);
  rv = crypto::derive_handshake_secret(
      handshake_secret.data(), handshake_secret.size(), conn_id,
      reinterpret_cast<const uint8_t *>(NGTCP2_QUIC_V1_SALT),
      str_size(NGTCP2_QUIC_V1_SALT));
 
  if (rv != 0) {
    std::cerr << "crypto::derive_handshake_secret() failed" << std::endl;
    return -1;
  }

  crypto::prf_sha256(hs_crypto_ctx_);
  crypto::aead_aes_128_gcm(hs_crypto_ctx_);

  rv = crypto::derive_client_handshake_secret(secret.data(), secret.size(),
                                              handshake_secret.data(),
                                              handshake_secret.size());
  if (rv != 0) {
    std::cerr << "crypto::derive_client_handshake_secret() failed" << std::endl;
    return -1;
  }

  std::array<uint8_t, 16> key, iv;

  auto keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), secret.data(), secret.size(), hs_crypto_ctx_);
  if (keylen < 0) {
    return -1;
  }

  auto ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), secret.data(), secret.size(), hs_crypto_ctx_);
  if (ivlen < 0) {
    return -1;
  }

  ngtcp2_conn_set_handshake_tx_keys(conn_, key.data(), keylen, iv.data(),
                                    ivlen);

  rv = crypto::derive_server_handshake_secret(secret.data(), secret.size(),
                                              handshake_secret.data(),
                                              handshake_secret.size());
  if (rv != 0) {
    std::cerr << "crypto::derive_server_handshake_secret() failed" << std::endl;
    return -1;
  }

  keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), secret.data(), secret.size(), hs_crypto_ctx_);
  if (keylen < 0) {
    return -1;
  }

  ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), secret.data(), secret.size(), hs_crypto_ctx_);
  if (ivlen < 0) {
    return -1;
  }

  ngtcp2_conn_set_handshake_rx_keys(conn_, key.data(), keylen, iv.data(),
                                    ivlen);
  
  ev_io_set(&wev_, fd_, EV_WRITE);
  ev_io_set(&rev_, fd_, EV_READ);

  ev_io_start(loop_, &rev_);
  ev_timer_again(loop_, &timer_);

  ev_signal_start(loop_, &sigintev_);

  return 0;
}

int Client::OnMigration(uint32_t* peer_address) {
  sockaddr_in6 remote_addr;
  in6_addr server_addr;
  for (int i = 0; i < 4; i++) 
    server_addr.s6_addr32[i] = peer_address[i];
  remote_addr.sin6_family = AF_INET6;
  remote_addr.sin6_port = remote_addr_.su.in6.sin6_port;
  remote_addr.sin6_addr = server_addr;
  
  remote_addr_.su.in6 = remote_addr;
  remote_addr_.len = sizeof(remote_addr);
  if (-1 == connect(fd_, &remote_addr_.su.sa, remote_addr_.len)) {
    std::cerr << "connect: " << strerror(errno) << std::endl;
    return -1;
  }
  return 1;
}

int Client::tls_handshake(bool initial) {
  ERR_clear_error();

  int rv;
  /* Note that SSL_SESSION_get_max_early_data() and
     SSL_get_max_early_data() return completely different value. */
  if (initial && resumption_ &&
      SSL_SESSION_get_max_early_data(SSL_get_session(ssl_))) {
    size_t nwrite;
    // OpenSSL returns error if SSL_write_early_data is called when
    // resumption is not attempted.  Sending empty string is a trick
    // to just early_data extension.
    rv = SSL_write_early_data(ssl_, "", 0, &nwrite);
    if (rv == 0) {
      auto err = SSL_get_error(ssl_, rv);
      switch (err) {
      case SSL_ERROR_SSL:
        std::cerr << "TLS handshake error: "
                  << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return -1;
      default:
        std::cerr << "TLS handshake error: " << err << std::endl;
        return -1;
      }
    }
  }

  rv = SSL_do_handshake(ssl_);
  if (!initial && resumption_) {
    if (SSL_get_early_data_status(ssl_) != SSL_EARLY_DATA_ACCEPTED) {
      std::cerr << "Early data was rejected by server" << std::endl;
      ngtcp2_conn_early_data_rejected(conn_);
    }
  }
  if (rv <= 0) {
    auto err = SSL_get_error(ssl_, rv);
    switch (err) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      return 0;
    case SSL_ERROR_SSL:
      std::cerr << "TLS handshake error: "
                << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
      return -1;
    default:
      std::cerr << "TLS handshake error: " << err << std::endl;
      return -1;
    }
  }
	
  std::cout << "tjx: conn completed!" << std::endl;
  ngtcp2_conn_handshake_completed(conn_);

  if (!config.quiet) {
    debug::print_indent();
    std::cerr << "; Negotiated cipher suite is " << SSL_get_cipher_name(ssl_)
              << std::endl;

    const unsigned char *alpn = nullptr;
    unsigned int alpnlen;

    SSL_get0_alpn_selected(ssl_, &alpn, &alpnlen);
    if (alpn) {
      debug::print_indent();
      std::cerr << "; Negotiated ALPN is ";
      std::cerr.write(reinterpret_cast<const char *>(alpn), alpnlen);
      std::cerr << std::endl;
    }
  }

  return 0;
}

int Client::read_tls() {
  ERR_clear_error();

  std::array<uint8_t, 4096> buf;
  size_t nread;

  for (;;) {
    auto outidx = chandshake_idx_;
    auto rv = SSL_read_ex(ssl_, buf.data(), buf.size(), &nread);
    if (rv == 1) {
      std::cerr << "Reads " << nread << " bytes from TLS stream 0."
                << std::endl;
      continue;
    }
    auto err = SSL_get_error(ssl_, 0);
    switch (err) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      return 0;
    case SSL_ERROR_SSL:
    case SSL_ERROR_ZERO_RETURN:
      std::cerr << "TLS read error: "
                << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
      if (chandshake_idx_ == outidx) {
        return NGTCP2_ERR_TLS_FATAL_ALERT_RECEIVED;
      }
      return NGTCP2_ERR_TLS_FATAL_ALERT_GENERATED;
    default:
      std::cerr << "TLS read error: " << err << std::endl;
      return NGTCP2_ERR_CALLBACK_FAILURE;
    }
  }
}

int Client::feed_data(uint8_t *data, size_t datalen) {
  int rv;

  rv = ngtcp2_conn_recv(conn_, data, datalen, util::timestamp());
  if (rv != 0) {
    std::cerr << "ngtcp2_conn_recv: " << ngtcp2_strerror(rv) << std::endl;
    if (rv != NGTCP2_ERR_TLS_DECRYPT) {
      disconnect(rv);
      return -1;
    }
  }
  if (ngtcp2_conn_in_draining_period(conn_)) {
    if (!config.quiet) {
      debug::print_timestamp();
      std::cerr << "QUIC connection has been closed by peer" << std::endl;
    }
    return -1;
  }

  return 0;
}

int Client::on_read() {
  std::array<uint8_t, 65536> buf;
  sockaddr_union su;
  socklen_t addrLen;
  addrLen=sizeof(struct sockaddr_in);

  for (;;) {
    auto nread =
        recvfrom(fd_, buf.data(), buf.size(), MSG_DONTWAIT, &su.sa, &addrLen);
    std::cout << "tjx:recv a udp packet!!!" << std::endl;

    if (nread == -1) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cerr << "recvfrom: " << strerror(errno) << std::endl;
      }
      break;
    }

    if (debug::packet_lost(config.rx_loss_prob)) {
      if (!config.quiet) {
        std::cerr << "** Simulated incoming packet loss **" << std::endl;
      }
      break;
    }

    if (feed_data(buf.data(), nread) != 0) {
      return -1;
    }
  }

  ev_timer_again(loop_, &timer_);

  return 0;
}

int Client::on_write() {
  if (sendbuf_.size() > 0) {
    auto rv = send_packet();
    if (rv != NETWORK_ERR_OK) {
      return rv;
    }
  }

  assert(sendbuf_.left() >= max_pktlen_);

  for (;;) {
    ssize_t n;
    if (ngtcp2_conn_bytes_in_flight(conn_) < MAX_BYTES_IN_FLIGHT) {
      n = ngtcp2_conn_write_pkt(conn_, sendbuf_.wpos(), max_pktlen_,
                                util::timestamp());
    } else {
      n = ngtcp2_conn_write_ack_pkt(conn_, sendbuf_.wpos(), max_pktlen_,
                                    util::timestamp());
    }
    if (n < 0) {
      std::cerr << "ngtcp2_conn_write_pkt: " << ngtcp2_strerror(n) << std::endl;
      disconnect(n);
      return -1;
    }
    if (n == 0) {
      break;
    }
    
    std::cout << "tjx: Send Packet!!" << std::endl;
    sendbuf_.push(n);

    auto rv = send_packet();
    if (rv == NETWORK_ERR_SEND_NON_FATAL) {
      break;
    }
    if (rv != NETWORK_ERR_OK) {
      return rv;
    }
  }

  for (auto &p : streams_) {
    auto &stream = p.second;
    auto &streambuf = stream->streambuf;
    auto &streambuf_idx = stream->streambuf_idx;

    for (auto it = std::begin(streambuf) + streambuf_idx;
         it != std::end(streambuf); ++it) {
      auto &v = *it;
      auto fin = stream->should_send_fin && it + 1 == std::end(streambuf);
      auto rv = on_write_stream(stream->stream_id, fin, v);
      if (rv != 0) {
        return rv;
      }
      if (v.size() > 0) {
        break;
      }
      ++streambuf_idx;
    }
  }

  schedule_retransmit();
  return 0;
}

int Client::on_write_stream(uint64_t stream_id, uint8_t fin, Buffer &data) {
  size_t ndatalen;

  for (;;) {
    if (ngtcp2_conn_bytes_in_flight(conn_) >= MAX_BYTES_IN_FLIGHT) {
      break;
    }

    auto n = ngtcp2_conn_write_stream(conn_, sendbuf_.wpos(), max_pktlen_,
                                      &ndatalen, stream_id, fin, data.rpos(),
                                      data.size(), util::timestamp());
    if (n < 0) {
      switch (n) {
      case NGTCP2_ERR_EARLY_DATA_REJECTED:
      case NGTCP2_ERR_STREAM_DATA_BLOCKED:
      case NGTCP2_ERR_STREAM_SHUT_WR:
      case NGTCP2_ERR_STREAM_NOT_FOUND: // This means that stream is
                                        // closed.
        return 0;
      }
      std::cerr << "ngtcp2_conn_write_stream: " << ngtcp2_strerror(n)
                << std::endl;
      disconnect(n);
      return -1;
    }

    data.seek(ndatalen);

    sendbuf_.push(n);

    auto rv = send_packet();
    if (rv != NETWORK_ERR_OK) {
      return rv;
    }

    if (data.size() == 0) {
      break;
    }
  }

  return 0;
}

void Client::schedule_retransmit() {
  auto expiry = ngtcp2_conn_earliest_expiry(conn_);
  if (expiry == 0) {
    return;
  }

  ev_tstamp t;
  auto now = util::timestamp();
  if (now >= expiry) {
    t = 0.;
  } else {
    t = static_cast<ev_tstamp>(expiry - now) / 1000000;
  }
  ev_timer_stop(loop_, &rttimer_);
  ev_timer_set(&rttimer_, t, 0.);
  ev_timer_start(loop_, &rttimer_);
}

int Client::write_client_handshake(const uint8_t *data, size_t datalen) {
  std::cout << "tjx: write_client_handshake! " << std::endl;
  chandshake_.emplace_back(data, datalen);
  return 0;
}

size_t Client::read_client_handshake(const uint8_t **pdest) {
  if (chandshake_idx_ == chandshake_.size()) {
    return 0;
  }
  const auto &v = chandshake_[chandshake_idx_++];
  *pdest = v.rpos();
  return v.size();
}

size_t Client::read_server_handshake(uint8_t *buf, size_t buflen) {
  auto n = std::min(buflen, shandshake_.size() - nsread_);
  std::copy_n(std::begin(shandshake_) + nsread_, n, buf);
  nsread_ += n;
  return n;
}

void Client::write_server_handshake(const uint8_t *data, size_t datalen) {
  std::copy_n(data, datalen, std::back_inserter(shandshake_));
}

int Client::setup_early_crypto_context() {
  int rv;

  rv = crypto::negotiated_prf(crypto_ctx_, ssl_);
  if (rv != 0) {
    return -1;
  }
  rv = crypto::negotiated_aead(crypto_ctx_, ssl_);
  if (rv != 0) {
    return -1;
  }

  auto length = EVP_MD_size(crypto_ctx_.prf);

  crypto_ctx_.secretlen = length;

  rv = crypto::export_early_secret(crypto_ctx_.tx_secret.data(),
                                   crypto_ctx_.secretlen, ssl_);
  if (rv != 0) {
    return -1;
  }

  std::array<uint8_t, 64> key, iv;

  auto keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), crypto_ctx_.tx_secret.data(),
      crypto_ctx_.secretlen, crypto_ctx_);
  if (keylen < 0) {
    return -1;
  }

  auto ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), crypto_ctx_.tx_secret.data(), crypto_ctx_.secretlen,
      crypto_ctx_);
  if (ivlen < 0) {
    return -1;
  }

  ngtcp2_conn_update_early_keys(conn_, key.data(), keylen, iv.data(), ivlen);

  ngtcp2_conn_set_aead_overhead(conn_, crypto::aead_max_overhead(crypto_ctx_));

  return 0;
}

int Client::setup_crypto_context() {
  int rv;

  rv = crypto::negotiated_prf(crypto_ctx_, ssl_);
  if (rv != 0) {
    return -1;
  }
  rv = crypto::negotiated_aead(crypto_ctx_, ssl_);
  if (rv != 0) {
    return -1;
  }

  auto length = EVP_MD_size(crypto_ctx_.prf);

  crypto_ctx_.secretlen = length;

  rv = crypto::export_client_secret(crypto_ctx_.tx_secret.data(),
                                    crypto_ctx_.secretlen, ssl_);
  if (rv != 0) {
    return -1;
  }

  std::array<uint8_t, 64> key{}, iv{};

  auto keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), crypto_ctx_.tx_secret.data(),
      crypto_ctx_.secretlen, crypto_ctx_);
  if (keylen < 0) {
    return -1;
  }

  auto ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), crypto_ctx_.tx_secret.data(), crypto_ctx_.secretlen,
      crypto_ctx_);
  if (ivlen < 0) {
    return -1;
  }

  ngtcp2_conn_update_tx_keys(conn_, key.data(), keylen, iv.data(), ivlen);

  rv = crypto::export_server_secret(crypto_ctx_.rx_secret.data(),
                                    crypto_ctx_.secretlen, ssl_);
  if (rv != 0) {
    return -1;
  }

  keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), crypto_ctx_.rx_secret.data(),
      crypto_ctx_.secretlen, crypto_ctx_);
  if (keylen < 0) {
    return -1;
  }

  ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), crypto_ctx_.rx_secret.data(), crypto_ctx_.secretlen,
      crypto_ctx_);
  if (ivlen < 0) {
    return -1;
  }

  ngtcp2_conn_update_rx_keys(conn_, key.data(), keylen, iv.data(), ivlen);

  ngtcp2_conn_set_aead_overhead(conn_, crypto::aead_max_overhead(crypto_ctx_));

  return 0;
}

ssize_t Client::hs_encrypt_data(uint8_t *dest, size_t destlen,
                                const uint8_t *plaintext, size_t plaintextlen,
                                const uint8_t *key, size_t keylen,
                                const uint8_t *nonce, size_t noncelen,
                                const uint8_t *ad, size_t adlen) {
  return crypto::encrypt(dest, destlen, plaintext, plaintextlen, hs_crypto_ctx_,
                         key, keylen, nonce, noncelen, ad, adlen);
}

ssize_t Client::hs_decrypt_data(uint8_t *dest, size_t destlen,
                                const uint8_t *ciphertext, size_t ciphertextlen,
                                const uint8_t *key, size_t keylen,
                                const uint8_t *nonce, size_t noncelen,
                                const uint8_t *ad, size_t adlen) {
  return crypto::decrypt(dest, destlen, ciphertext, ciphertextlen,
                         hs_crypto_ctx_, key, keylen, nonce, noncelen, ad,
                         adlen);
}

ssize_t Client::encrypt_data(uint8_t *dest, size_t destlen,
                             const uint8_t *plaintext, size_t plaintextlen,
                             const uint8_t *key, size_t keylen,
                             const uint8_t *nonce, size_t noncelen,
                             const uint8_t *ad, size_t adlen) {
  return crypto::encrypt(dest, destlen, plaintext, plaintextlen, crypto_ctx_,
                         key, keylen, nonce, noncelen, ad, adlen);
}

ssize_t Client::decrypt_data(uint8_t *dest, size_t destlen,
                             const uint8_t *ciphertext, size_t ciphertextlen,
                             const uint8_t *key, size_t keylen,
                             const uint8_t *nonce, size_t noncelen,
                             const uint8_t *ad, size_t adlen) {
  return crypto::decrypt(dest, destlen, ciphertext, ciphertextlen, crypto_ctx_,
                         key, keylen, nonce, noncelen, ad, adlen);
}

ngtcp2_conn *Client::conn() const { return conn_; }

int Client::send_packet() {
  if (debug::packet_lost(config.tx_loss_prob)) {
    if (!config.quiet) {
      std::cerr << "** Simulated outgoing packet loss **" << std::endl;
    }
    sendbuf_.reset();
    return NETWORK_ERR_OK;
  }

  int eintr_retries = 5;
  ssize_t nwrite = 0;

  do {
    nwrite = sendto(fd_, sendbuf_.rpos(), sendbuf_.size(), 0, (struct sockaddr*) &remote_addr_.su, sizeof(remote_addr_.su));
  } while ((nwrite == -1) && (errno == EINTR) && (eintr_retries-- > 0));

  if (nwrite == -1) {
    switch (errno) {
    case EAGAIN:
    case EINTR:
    case 0:
      return NETWORK_ERR_SEND_NON_FATAL;
    default:
      std::cerr << "send: " << strerror(errno) << std::endl;
      return NETWORK_ERR_SEND_FATAL;
    }
  }

  assert(static_cast<size_t>(nwrite) == sendbuf_.size());
  sendbuf_.reset();

  return NETWORK_ERR_OK;
}

int Client::start_interactive_input() {
  int rv;

  std::cerr << "Interactive session started.  Hit Ctrl-D to end the session."
            << std::endl;

  ev_io_set(&stdinrev_, datafd_, EV_READ);
  ev_io_start(loop_, &stdinrev_);

  uint64_t stream_id;

  rv = ngtcp2_conn_open_bidi_stream(conn_, &stream_id, nullptr);
  if (rv != 0) {
    std::cerr << "ngtcp2_conn_open_bidi_stream: " << ngtcp2_strerror(rv)
              << std::endl;
    if (rv == NGTCP2_ERR_STREAM_ID_BLOCKED) {
      return 0;
    }
    return -1;
  }

  std::cerr << "The stream " << stream_id << " has opened." << std::endl;

  last_stream_id_ = stream_id;

  auto stream = std::make_unique<Stream>(stream_id);

  streams_.emplace(stream_id, std::move(stream));

  return 0;
}

int Client::send_interactive_input() {
  ssize_t nread;
  std::array<uint8_t, 1_k> buf;
  sockaddr_union su;
  socklen_t addrLen;
  addrLen=sizeof(struct sockaddr_in);
  

  while ((nread = recvfrom(datafd_, buf.data(), buf.size(), 0, &su.sa, &addrLen)) == -1 &&
         errno == EINTR)
    ;
  if (nread == -1) {
    return stop_interactive_input();
  }
  if (nread == 0) {
    return stop_interactive_input();
  }

  // TODO fix this
  assert(!streams_.empty());

  auto &stream = streams_[last_stream_id_];

  stream->streambuf.emplace_back(buf.data(), nread);

  ev_feed_event(loop_, &wev_, EV_WRITE);

  return 0;
}

int Client::stop_interactive_input() {
  assert(!streams_.empty());

  auto &stream = (*std::begin(streams_)).second;

  stream->should_send_fin = true;
  if (stream->streambuf.empty()) {
    stream->streambuf.emplace_back();
  }
  ev_io_stop(loop_, &stdinrev_);

  std::cerr << "Interactive session has ended." << std::endl;

  ev_feed_event(loop_, &wev_, EV_WRITE);

  return 0;
}

int Client::handle_error(int liberr) {
  if (!conn_ || ngtcp2_conn_in_closing_period(conn_)) {
    return 0;
  }

  sendbuf_.reset();
  assert(sendbuf_.left() >= max_pktlen_);

  auto n = ngtcp2_conn_write_connection_close(
      conn_, sendbuf_.wpos(), max_pktlen_,
      ngtcp2_err_infer_quic_transport_error_code(liberr));
  if (n < 0) {
    std::cerr << "ngtcp2_conn_write_connection_close: " << ngtcp2_strerror(n)
              << std::endl;
    return -1;
  }

  sendbuf_.push(n);

  return send_packet();
}

namespace {
size_t remove_tx_stream_data(std::deque<Buffer> &d, size_t &idx,
                             uint64_t &tx_offset, uint64_t offset) {
  size_t len = 0;
  for (; !d.empty() && tx_offset + d.front().bufsize() <= offset;) {
    --idx;
    tx_offset += d.front().bufsize();
    len += d.front().bufsize();
    d.pop_front();
  }
  return len;
}
} // namespace

int Client::remove_tx_stream_data(uint64_t stream_id, uint64_t offset,
                                  size_t datalen) {
  if (stream_id == 0) {
    ::remove_tx_stream_data(chandshake_, chandshake_idx_, tx_stream0_offset_,
                            offset + datalen);
    return 0;
  }

  auto it = streams_.find(stream_id);
  if (it == std::end(streams_)) {
    std::cerr << "Stream " << stream_id << "not found" << std::endl;
    return 0;
  }
  auto &stream = (*it).second;
  ::remove_tx_stream_data(stream->streambuf, stream->streambuf_idx,
                          stream->tx_stream_offset, offset + datalen);

  return 0;
}

void Client::on_stream_close(uint64_t stream_id) {
  auto it = streams_.find(stream_id);

  if (it == std::end(streams_)) {
    return;
  }

  if (config.interactive) {
    ev_io_stop(loop_, &stdinrev_);
  }

  streams_.erase(it);
}

namespace {
int write_transport_params(const char *path,
                           const ngtcp2_transport_params *params) {
  auto f = std::ofstream(path);
  if (!f) {
    return -1;
  }

  f << "initial_max_stream_id_bidi=" << params->initial_max_stream_id_bidi
    << "\n"
    << "initial_max_stream_id_uni=" << params->initial_max_stream_id_uni << "\n"
    << "initial_max_stream_data=" << params->initial_max_stream_data << "\n"
    << "initial_max_data=" << params->initial_max_data << "\n";

  f.close();
  if (!f) {
    return -1;
  }

  return 0;
}
} // namespace

namespace {
int read_transport_params(const char *path, ngtcp2_transport_params *params) {
  auto f = std::ifstream(path);
  if (!f) {
    return -1;
  }

  for (std::string line; std::getline(f, line);) {
    if (util::istarts_with_l(line, "initial_max_stream_id_bidi=")) {
      params->initial_max_stream_id_bidi = strtoul(
          line.c_str() + str_size("initial_max_stream_id_bidi="), nullptr, 10);
    } else if (util::istarts_with_l(line, "initial_max_stream_id_uni=")) {
      params->initial_max_stream_id_uni = strtoul(
          line.c_str() + str_size("initial_max_stream_id_uni="), nullptr, 10);
    } else if (util::istarts_with_l(line, "initial_max_stream_data=")) {
      params->initial_max_stream_data = strtoul(
          line.c_str() + str_size("initial_max_stream_data="), nullptr, 10);
    } else if (util::istarts_with_l(line, "initial_max_data=")) {
      params->initial_max_data =
          strtoul(line.c_str() + str_size("initial_max_data="), nullptr, 10);
    }
  }

  return 0;
}
} // namespace

void Client::handle_early_data() {
  if (!resumption_ || setup_early_crypto_context() != 0) {
    return;
  }

  if (config.tp_file) {
    ngtcp2_transport_params params;
    if (read_transport_params(config.tp_file, &params) != 0) {
      std::cerr << "Could not read transport parameters from " << config.tp_file
                << std::endl;
    } else {
      ngtcp2_conn_set_early_remote_transport_params(conn_, &params);
    }
  }

  if (config.interactive || datafd_ == -1) {
    return;
  }

  make_stream_early();
}

void Client::make_stream_early() {
  int rv;

  if (nstreams_done_ >= config.nstreams) {
    return;
  }

  ++nstreams_done_;

  uint64_t stream_id;
  rv = ngtcp2_conn_open_bidi_stream(conn_, &stream_id, nullptr);
  if (rv != 0) {
    std::cerr << "ngtcp2_conn_open_bidi_stream: " << ngtcp2_strerror(rv)
              << std::endl;
    return;
  }

  auto stream = std::make_unique<Stream>(stream_id);
  stream->buffer_file();
  streams_.emplace(stream_id, std::move(stream));
}

int Client::on_extend_max_stream_id(uint64_t max_stream_id) {
  int rv;

  if (config.interactive) {
    if (last_stream_id_ != 0) {
      return 0;
    }
    if (start_interactive_input() != 0) {
      return -1;
    }

    return 0;
  }

  if (datafd_ != -1) {
    for (; nstreams_done_ < config.nstreams; ++nstreams_done_) {
      uint64_t stream_id;

      rv = ngtcp2_conn_open_bidi_stream(conn_, &stream_id, nullptr);
      if (rv != 0) {
        assert(NGTCP2_ERR_STREAM_ID_BLOCKED == rv);
        break;
      }

      last_stream_id_ = stream_id;

      auto stream = std::make_unique<Stream>(stream_id);
      stream->buffer_file();

      streams_.emplace(stream_id, std::move(stream));
    }
    return 0;
  }

  return 0;
}

namespace {
int transport_params_add_cb(SSL *ssl, unsigned int ext_type,
                            unsigned int content, const unsigned char **out,
                            size_t *outlen, X509 *x, size_t chainidx, int *al,
                            void *add_arg) {
  int rv;
  auto c = static_cast<Client *>(SSL_get_app_data(ssl));
  auto conn = c->conn();

  ngtcp2_transport_params params;
  std::cout << "tjx: client_add_transport_params!" << std::endl;

  rv = ngtcp2_conn_get_local_transport_params(
      conn, &params, NGTCP2_TRANSPORT_PARAMS_TYPE_CLIENT_HELLO);
  if (rv != 0) {
    *al = SSL_AD_INTERNAL_ERROR;
    return -1;
  }

  constexpr size_t bufsize = 128;
  auto buf = std::make_unique<uint8_t[]>(bufsize);

  auto nwrite = ngtcp2_encode_transport_params(
      buf.get(), bufsize, NGTCP2_TRANSPORT_PARAMS_TYPE_CLIENT_HELLO, &params);
  std::cout << "tjx: transport_params length: " << nwrite << std::endl;
  if (nwrite < 0) {
    std::cerr << "ngtcp2_encode_transport_params: " << ngtcp2_strerror(nwrite)
              << std::endl;
    *al = SSL_AD_INTERNAL_ERROR;
    return -1;
  }

  *out = buf.release();
  *outlen = static_cast<size_t>(nwrite);

  return 1;
}
} // namespace

namespace {
void transport_params_free_cb(SSL *ssl, unsigned int ext_type,
                              unsigned int context, const unsigned char *out,
                              void *add_arg) {
  delete[] const_cast<unsigned char *>(out);
}
} // namespace

namespace {
int transport_params_parse_cb(SSL *ssl, unsigned int ext_type,
                              unsigned int context, const unsigned char *in,
                              size_t inlen, X509 *x, size_t chainidx, int *al,
                              void *parse_arg) {
  auto c = static_cast<Client *>(SSL_get_app_data(ssl));
  auto conn = c->conn();

  int rv;

  ngtcp2_transport_params params;
  int param_type = context == SSL_EXT_TLS1_3_ENCRYPTED_EXTENSIONS
                       ? NGTCP2_TRANSPORT_PARAMS_TYPE_ENCRYPTED_EXTENSIONS
                       : NGTCP2_TRANSPORT_PARAMS_TYPE_NEW_SESSION_TICKET;

  std::cout << "tjx: decode_transport_params! type: " << param_type << std::endl;
  rv = ngtcp2_decode_transport_params(&params, param_type, in, inlen);
  if (rv != 0) {
    std::cerr << "ngtcp2_decode_transport_params: " << ngtcp2_strerror(rv)
              << std::endl;
    *al = SSL_AD_ILLEGAL_PARAMETER;
    return -1;
  }
  std::cout << "tjx: decode_transport_params  server_ip: " << params.server_unicast_ip << std::endl;

  if ((param_type == 1) && (params.server_unicast_ip != 0)) {
    std::cout << "tjx: begin to migration" << std::endl;
    c->OnMigration(params.server_unicast_ip);
  }

  if (!config.quiet) {
    debug::print_indent();
    std::cerr << "; TransportParameter received in "
              << (context == SSL_EXT_TLS1_3_ENCRYPTED_EXTENSIONS
                      ? "EncryptedExtensions"
                      : "NewSessionTicket")
              << std::endl;
    debug::print_transport_params(&params, param_type);
  }

  if (context == SSL_EXT_TLS1_3_ENCRYPTED_EXTENSIONS) {
    rv = ngtcp2_conn_set_remote_transport_params(conn, param_type, &params);
    if (rv != 0) {
      *al = SSL_AD_ILLEGAL_PARAMETER;
      return -1;
    }
  } else if (config.tp_file) {
    if (write_transport_params(config.tp_file, &params) != 0) {
      std::cerr << "Could not write transport parameters in " << config.tp_file
                << std::endl;
    }
  }

  return 1;
}
} // namespace

namespace {
int new_session_cb(SSL *ssl, SSL_SESSION *session) {
  if (SSL_SESSION_get_max_early_data(session) !=
      std::numeric_limits<uint32_t>::max()) {
    // TODO Find a way to send CONNECTION_CLOSE with
    // PROTOCOL_VIOLATION in this case.
    std::cerr << "max_early_data is not 0xffffffff" << std::endl;
  }
  auto f = BIO_new_file(config.session_file, "w");
  if (f == nullptr) {
    std::cerr << "Could not write TLS session in " << config.session_file
              << std::endl;
    return 0;
  }

  PEM_write_bio_SSL_SESSION(f, session);
  BIO_free(f);

  return 0;
}
} // namespace

namespace {
SSL_CTX *create_ssl_ctx() {
  auto ssl_ctx = SSL_CTX_new(TLS_method());

  SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_3_VERSION);
  SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_3_VERSION);

  SSL_CTX_set_default_verify_paths(ssl_ctx);

  if (SSL_CTX_set_cipher_list(ssl_ctx, config.ciphers) != 1) {
    std::cerr << "SSL_CTX_set_cipher_list: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_set1_groups_list(ssl_ctx, config.groups) != 1) {
    std::cerr << "SSL_CTX_set1_groups_list failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_add_custom_ext(
          ssl_ctx, NGTCP2_TLSEXT_QUIC_TRANSPORT_PARAMETERS,
          SSL_EXT_CLIENT_HELLO | SSL_EXT_TLS1_3_ENCRYPTED_EXTENSIONS |
              SSL_EXT_TLS1_3_NEW_SESSION_TICKET,
          transport_params_add_cb, transport_params_free_cb, nullptr,
          transport_params_parse_cb, nullptr) != 1) {
    std::cerr << "SSL_CTX_add_custom_ext(NGTCP2_TLSEXT_QUIC_TRANSPORT_"
                 "PARAMETERS) failed: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    exit(EXIT_FAILURE);
  }

  if (config.session_file) {
    SSL_CTX_set_session_cache_mode(
        ssl_ctx, SSL_SESS_CACHE_CLIENT | SSL_SESS_CACHE_NO_INTERNAL_STORE);
    SSL_CTX_sess_set_new_cb(ssl_ctx, new_session_cb);
  }

  return ssl_ctx;
}
} // namespace



namespace {
int run(Client &c, const char *addr, const char *port) {
  Address remote_addr;

  std::cout << "tjx: input_addr: " << addr << std::endl;
  auto fd = create_sock(remote_addr, addr, port);
  if (fd == -1) {
    return -1;
  }

  std::cout << "tjx: remote_addr_port: " << remote_addr.su.in6.sin6_port << std::endl;

  if (c.init(fd, remote_addr, addr, config.fd, config.version) != 0) {
    return -1;
  }

  c.on_write();

  ev_run(EV_DEFAULT, 0);

  return 0;
}
} // namespace

namespace {
void close(Client &c) {
  c.disconnect();

  c.close();
}
} // namespace

namespace {
void print_usage() {
  std::cerr << "Usage: client [OPTIONS] <ADDR> <PORT>" << std::endl;
}
} // namespace

namespace {
void config_set_default(Config &config) {
  config = Config{};
  config.tx_loss_prob = 0.;
  config.rx_loss_prob = 0.;
  config.fd = -1;
  config.ciphers = "TLS13-AES-128-GCM-SHA256:TLS13-AES-256-GCM-SHA384:TLS13-"
                   "CHACHA20-POLY1305-SHA256";
  config.groups = "P-256:X25519:P-384:P-521";
  config.nstreams = 1;
  config.data = nullptr;
  config.datalen = 0;
  config.version = NGTCP2_PROTO_VER_D8;
  config.timeout = 30;
}
} // namespace

namespace {
void print_help() {
  print_usage();

  config_set_default(config);

  std::cout << R"(
  <ADDR>      Remote server address
  <PORT>      Remote server port
Options:
  -t, --tx-loss=<P>
              The probability of losing outgoing packets.  <P> must be
              [0.0, 1.0],  inclusive.  0.0 means no  packet loss.  1.0
              means 100% packet loss.
  -r, --rx-loss=<P>
              The probability of losing incoming packets.  <P> must be
              [0.0, 1.0],  inclusive.  0.0 means no  packet loss.  1.0
              means 100% packet loss.
  -i, --interactive
              Read input from stdin, and send them as STREAM data.
  -d, --data=<PATH>
              Read data from <PATH>, and send them as STREAM data.
  -n, --nstreams=<N>
              When used with --data,  this option specifies the number
              of streams to send the data specified by --data.
  -v, --version=<HEX>
              Specify QUIC version to use in hex string.
              Default: )"
            << std::hex << "0x" << config.version << std::dec << R"(
  -q, --quiet Suppress debug output.
  --timeout=<T>
              Specify idle timeout in seconds.
              Default: )"
            << config.timeout << R"(
  --ciphers=<CIPHERS>
              Specify the cipher suite list to enable.
              Default: )" << config.ciphers << R"(
  --groups=<GROUPS>
              Specify the supported groups.
              Default: )" << config.groups
            << R"(
  --session-file=<PATH>
              Read/write  TLS session  from/to  <PATH>.   To resume  a
              session, the previous session must be supplied with this
              option.
  --tp-file=<PATH>
              Read/write QUIC transport parameters from/to <PATH>.  To
              send 0-RTT data, the  transport parameters received from
              the previous session must be supplied with this option.
  -h, --help  Display this help and exit.
)";
}
} // namespace

namespace {
#define SHA256_ROTL(a,b) (((a>>(32-b))&(0x7fffffff>>(31-b)))|(a<<b))  
#define SHA256_SR(a,b) ((a>>b)&(0x7fffffff>>(b-1)))  
#define SHA256_Ch(x,y,z) ((x&y)^((~x)&z))  
#define SHA256_Maj(x,y,z) ((x&y)^(x&z)^(y&z))  
#define SHA256_E0(x) (SHA256_ROTL(x,30)^SHA256_ROTL(x,19)^SHA256_ROTL(x,10))  
#define SHA256_E1(x) (SHA256_ROTL(x,26)^SHA256_ROTL(x,21)^SHA256_ROTL(x,7))  
#define SHA256_O0(x) (SHA256_ROTL(x,25)^SHA256_ROTL(x,14)^SHA256_SR(x,3))  
#define SHA256_O1(x) (SHA256_ROTL(x,15)^SHA256_ROTL(x,13)^SHA256_SR(x,10))
char* StrSHA256(const char* str, long long length, char* sha256){  
    char *pp, *ppend;  
    long l, i, W[64], T1, T2, A, B, C, D, E, F, G, H, H0, H1, H2, H3, H4, H5, H6, H7;  
    H0 = 0x6a09e667, H1 = 0xbb67ae85, H2 = 0x3c6ef372, H3 = 0xa54ff53a;  
    H4 = 0x510e527f, H5 = 0x9b05688c, H6 = 0x1f83d9ab, H7 = 0x5be0cd19;  
    long K[64] = {  
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,  
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,  
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,  
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,  
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,  
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,  
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,  
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,  
    };  
    l = length + ((length % 64 > 56) ? (128 - length % 64) : (64 - length % 64));  
    if (!(pp = (char*)malloc((unsigned long)l))) return 0;  
    for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = str[i], i++);  
    for (pp[i + 3 - 2 * (i % 4)] = 128, i++; i < l; pp[i + 3 - 2 * (i % 4)] = 0, i++);  
    *((long*)(pp + l - 4)) = length << 3;  
    *((long*)(pp + l - 8)) = length >> 29;  
    for (ppend = pp + l; pp < ppend; pp += 64){  
        for (i = 0; i < 16; W[i] = ((long*)pp)[i], i++);  
        for (i = 16; i < 64; W[i] = (SHA256_O1(W[i - 2]) + W[i - 7] + SHA256_O0(W[i - 15]) + W[i - 16]), i++);  
        A = H0, B = H1, C = H2, D = H3, E = H4, F = H5, G = H6, H = H7;  
        for (i = 0; i < 64; i++){  
            T1 = H + SHA256_E1(E) + SHA256_Ch(E, F, G) + K[i] + W[i];  
            T2 = SHA256_E0(A) + SHA256_Maj(A, B, C);  
            H = G, G = F, F = E, E = D + T1, D = C, C = B, B = A, A = T1 + T2;  
        }  
        H0 += A, H1 += B, H2 += C, H3 += D, H4 += E, H5 += F, H6 += G, H7 += H;  
    }  
    free(pp - l);  
    sprintf(sha256, "%08X%08X%08X%08X%08X%08X%08X%08X", H0, H1, H2, H3, H4, H5, H6, H7);  
    return sha256;  
}  
}

namespace {
const char* get_hashed_ip(const char *url) {
  url = "hello world!";
  long long len = strlen(url);
  char hashed_result[256];
  StrSHA256(url, len, hashed_result);
  std::cout << hashed_result << std::endl;
  url = "www.baidu.com";
  len = strlen(url);
  StrSHA256(url, len, hashed_result);
  std::cout << hashed_result << std::endl;
  return "::1";
} 
}

int main(int argc, char **argv) {
  config_set_default(config);
  char *data_path = nullptr;

  for (;;) {
    static int flag = 0;
    constexpr static option long_opts[] = {
        {"help", no_argument, nullptr, 'h'},
        {"tx-loss", required_argument, nullptr, 't'},
        {"rx-loss", required_argument, nullptr, 'r'},
        {"interactive", no_argument, nullptr, 'i'},
        {"data", required_argument, nullptr, 'd'},
        {"nstreams", required_argument, nullptr, 'n'},
        {"version", required_argument, nullptr, 'v'},
        {"quiet", no_argument, nullptr, 'q'},
        {"ciphers", required_argument, &flag, 1},
        {"groups", required_argument, &flag, 2},
        {"timeout", required_argument, &flag, 3},
        {"session-file", required_argument, &flag, 4},
        {"tp-file", required_argument, &flag, 5},
        {nullptr, 0, nullptr, 0},
    };

    auto optidx = 0;
    auto c = getopt_long(argc, argv, "d:hin:qr:t:v:", long_opts, &optidx);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 'd':
      // --data
      data_path = optarg;
      break;
    case 'h':
      // --help
      print_help();
      exit(EXIT_SUCCESS);
    case 'n':
      // --streams
      config.nstreams = strtol(optarg, nullptr, 10);
      break;
    case 'q':
      // -quiet
      config.quiet = true;
      break;
    case 'r':
      // --rx-loss
      config.rx_loss_prob = strtod(optarg, nullptr);
      break;
    case 't':
      // --tx-loss
      config.tx_loss_prob = strtod(optarg, nullptr);
      break;
    case 'i':
      // --interactive
      config.fd = fileno(stdin);
      config.interactive = true;
      break;
    case 'v':
      // --version
      config.version = strtol(optarg, nullptr, 16);
      break;
    case '?':
      print_usage();
      exit(EXIT_FAILURE);
    case 0:
      switch (flag) {
      case 1:
        // --ciphers
        config.ciphers = optarg;
        break;
      case 2:
        // --groups
        config.groups = optarg;
        break;
      case 3:
        // --timeout
        config.timeout = strtol(optarg, nullptr, 10);
        break;
      case 4:
        // --session-file
        config.session_file = optarg;
        break;
      case 5:
        // --tp-file
        config.tp_file = optarg;
        break;
      }
      break;
    default:
      break;
    };
  }

  if (argc - optind < 2) {
    std::cerr << "Too few arguments" << std::endl;
    print_usage();
    exit(EXIT_FAILURE);
  }

  if (data_path && config.interactive) {
    std::cerr
        << "interactive, data: Exclusive options are specified at the same time"
        << std::endl;
    exit(EXIT_FAILURE);
  }

  if (data_path) {
    auto fd = open(data_path, O_RDONLY);
    if (fd == -1) {
      std::cerr << "data: Could not open file " << data_path << ": "
                << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
      std::cerr << "data: Could not stat file " << data_path << ": "
                << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    config.fd = fd;
    config.datalen = st.st_size;
    config.data = static_cast<uint8_t *>(
        mmap(nullptr, config.datalen, PROT_READ, MAP_SHARED, fd, 0));
  }

  auto addr = get_hashed_ip(argv[optind++]);
  // auto addr = argv[optind++];
  auto port = argv[optind++];

  auto ssl_ctx = create_ssl_ctx();
  auto ssl_ctx_d = defer(SSL_CTX_free, ssl_ctx);

  auto ev_loop_d = defer(ev_loop_destroy, EV_DEFAULT);

  debug::reset_timestamp();

  if (isatty(STDOUT_FILENO)) {
    debug::set_color_output(true);
  }

  Client c(EV_DEFAULT, ssl_ctx);

  if (run(c, addr, port) != 0) {
    exit(EXIT_FAILURE);
  }

  close(c);

  return EXIT_SUCCESS;
}
