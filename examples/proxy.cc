#include <ev.h>
#include <array>
#include <iostream>
#include <getopt.h>
#include <unistd.h>
#include "network.h"
#include "template.h"

#define FD_SIZE 20
#define ETHER_TYPE    0x0800
ev_io revs_[FD_SIZE];
int fds_[FD_SIZE];
int read_index = 0;
ev_signal sigintev_;
int wfd;
struct ev_loop *loop_ = EV_DEFAULT;


void siginthandler(struct ev_loop *loop, ev_signal *watcher, int revents) {
  ev_break(loop, EVBREAK_ALL);
}

void sreadcb(struct ev_loop *loop, ev_io *w, int revents) {
  ngtcp2::sockaddr_union su;
  socklen_t addrlen = sizeof(su);
  int fd = *(int *) (w->data);
  std::array<uint8_t, 64_k> buf;
  auto nread = recvfrom(fd, buf.data(), buf.size(), MSG_DONTWAIT, &su.sa, &addrlen);
  if (nread == -1) {
    std::cerr << "recvfrom " << fd << " : " << strerror(errno) << std::endl;
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
  char str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
  std::cerr << "send packet to " << str << ":" << ntohs(sa.sin_port) << std::endl;
  if (sendto(wfd, iph, ntohs(iph->tot_len), 0, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("Failed to forward ip packet");
  } else {
    std::cerr << "Forwarded to server, " << ntohs(iph->tot_len) << " bytes" << std::endl;
  }
}

int listen(char *interface, bool listen) {
  int fd = -1;
  if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    std::cerr << "Could not bind" << std::endl;
    close(fd);
    return -1;
  }
  int val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, interface, sizeof(interface)) == -1) {
    std::cerr << "Failed to bind interface: " << interface << ", " << strerror(errno) << std::endl;
    close(fd);
    return -1;
  }
  if (!listen) {
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)) == -1) {
      std::cerr << "Failed to set IP_HDRINCL: " << interface << ", " << strerror(errno) << std::endl;
      close(fd);
      return -1;
    }
  }
  if (listen) {
    fds_[read_index] = fd;
    ev_io_init(revs_ + read_index, sreadcb, 0, EV_READ);
    ev_signal_init(&sigintev_, siginthandler, SIGINT);
    ev_io_set(revs_ + read_index, fd, EV_READ);
    ev_io_start(loop_, revs_ + read_index);
    ev_signal_start(loop_, &sigintev_);
    revs_[read_index].data = fds_ + read_index;
    read_index++;
    printf("Listen on interface: %s, %d\n", interface, fd);
  } else {
    printf("Bind to interface: %s, %d\n", interface, fd);
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
      if (!strncmp(tmp->ifa_name, "ser", 3)) {
        listen(tmp->ifa_name, true);
      }
    }
    tmp = tmp->ifa_next;
  }
  freeifaddrs(addrs);
  ev_run(EV_DEFAULT, 0);

  for (auto rev: revs_) {
    ev_io_stop(loop_, &rev);
  }
  ev_signal_stop(loop_, &sigintev_);

  return EXIT_SUCCESS;
}