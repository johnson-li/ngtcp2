#include <ev.h>
#include <array>
#include <iostream>
#include <getopt.h>
#include <unistd.h>
#include "network.h"
#include "template.h"

#define ETHER_TYPE    0x0800
ev_io revs_[20];
int read_index = 0;
ev_signal sigintev_;
int wfd;


void siginthandler(struct ev_loop *loop, ev_signal *watcher, int revents) {
  ev_break(loop, EVBREAK_ALL);
}

void sreadcb(struct ev_loop *loop, ev_io *w, int revents) {
  ngtcp2::sockaddr_union su;
  socklen_t addrlen = sizeof(su);
  auto fd = *static_cast<int *>(w->data);
  std::array<uint8_t, 64_k> buf;
  auto nread = recvfrom(fd, buf.data(), buf.size(), MSG_DONTWAIT, &su.sa, &addrlen);
  if (nread == -1) {
    std::cerr << "recvfrom: " << strerror(errno) << std::endl;
    return;
  }
  uint8_t *data = buf.data();
  ether_header *eh = (ether_header *) data;
  iphdr *iph = (iphdr * )(data + sizeof(ether_header));
  udphdr *udph = (udphdr * )(data + sizeof(iphdr) + sizeof(ether_header));
  uint8_t *quic = data + sizeof(udphdr) + sizeof(iphdr) + sizeof(ether_header);
  nread -= sizeof(udphdr) + sizeof(iphdr) + sizeof(ether_header);

  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = udph->dest;
  sa.sin_addr.s_addr = iph->daddr;
  if (sendto(wfd, iph, ntohs(iph->tot_len), 0, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("Failed to forward ip packet");
  } else {
    std::cerr << "Forwarded to server, " << ntohs(iph->tot_len) << " bytes" << std::endl;
  }
}

int listen(char *interface, bool listen) {
  int fd = -1;
  struct ev_loop *loop_ = EV_DEFAULT;
  if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
    std::cerr << "Could not bind" << std::endl;
    close(fd);
    return -1;
  }
  int val = 1;
//  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, static_cast<socklen_t>(sizeof(val))) == -1 ||
//      setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)) == -1) {
//    std::cerr << "Could not set reuse addr" << std::endl;
//    close(fd);
//    return -1;
//  }
  if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, interface, sizeof(interface)) == -1) {
    std::cerr << "Failed to bind interface: " << interface << ", " << strerror(errno) << std::endl;
    close(fd);
    return -1;
  }
  if (listen) {
    ev_io_init(revs_ + read_index, sreadcb, 0, EV_READ);
    ev_signal_init(&sigintev_, siginthandler, SIGINT);
    ev_io_set(revs_ + read_index, fd, EV_READ);
    ev_io_start(loop_, revs_ + read_index);
    ev_signal_start(loop_, &sigintev_);
    revs_[read_index].data = &fd;
    read_index++;
    printf("Listen on interface: %s\n", interface);
  } else {
    printf("Bind to interface: %s\n", interface);
  }
  return fd;
}

int main(int argc, char **argv) {
  if (argc - optind < 1) {
    std::cerr << "Too few arguments" << std::endl;
    std::cerr << "proxy interface" << std::endl;
    exit(EXIT_FAILURE);
  }

  auto interface = argv[optind++];
  wfd = listen(interface, false);

  struct ifaddrs *addrs, *tmp;
  getifaddrs(&addrs);
  tmp = addrs;
  while (tmp) {
    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET) {
      if (!strncmp(tmp->ifa_name, "server", 6)) {
        listen(tmp->ifa_name, true);
      }
    }
    tmp = tmp->ifa_next;
  }
  freeifaddrs(addrs);
  ev_run(EV_DEFAULT, 0);
  return EXIT_SUCCESS;
}