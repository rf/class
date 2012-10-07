#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
using namespace std;

#include "store.h"
#include "dns.h"

// Group member: Russ Frank, working alone

int
main (int argc, char ** argv) {
  int rc;

  char * optstr = "usage: %s [-p port] [-f hostfile]";
  extern char * optarg;
  extern int optind;
  int c;

  char * filename = "hosts.txt";
  char * portstr = "20000";

  while ((c = getopt(argc, argv, "p:f:")) != -1) {
    switch (c) {
      case 'p':
        portstr = optarg;
      break;

      case 'f':
        filename = optarg;
      break;
    }
  }

  int port = atoi(portstr);
  auto names = get_names(filename);

  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (fd < 0) {
    perror("cannot create socket");
    return 1;
  }

  struct sockaddr_in addr;
  memset((char *) & addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  rc = bind(fd, (struct sockaddr *) & addr, sizeof(addr));
  if (rc < 0) {
    perror("bind failed");
    return 2;
  }

  int operate = 1, recvlen, q_len, res_len;
  char buf[1024];
  struct sockaddr_in remaddr;
  socklen_t addrlen = sizeof(remaddr);
  while (operate) {
    recvlen = recvfrom(
      fd, 
      buf, 
      1024, 
      0, 
      (struct sockaddr *) & remaddr, 
      & addrlen
    );

    int type, klass;

    auto parsed = parse_dns_req(buf, &q_len, &type, &klass);
    res_len = 0;

    char * ptr = buf;

    dns_header * h = (dns_header *) ptr;
    h->qr = 1;
    res_len += sizeof(dns_header) + q_len;

    if (type == 1 && klass == 1 && names.count(parsed) == 1) {
      cout << "Returning: " << names[parsed] << endl;

      h->aa = 1;
      h->tc = 0;
      h->ra = 0;
      h->rcode = 0;
      h->an_count = htons(1);

      // Advance past the dns header and query
      ptr += sizeof(dns_header) + q_len;

      memcpy(ptr, parsed.c_str(), strlen(parsed.c_str()) + 1);
      ptr += strlen(parsed.c_str()) + 1;
      res_len += strlen(parsed.c_str()) + 1;

      dns_ip_answer * a = (dns_ip_answer *) ptr;
      memset(a, 0, sizeof(dns_ip_answer));
      a->type = htons(1);
      a->klass = htons(1);;
      a->data_len = htons(4);
      a->ttl = 0;
      a->address = (strip_to_intip(names[parsed]));

      res_len += sizeof(dns_ip_answer);
    }

    else {
      // If we don't know about the domain, mark the response as NXDOMAIN
      if (names.count(parsed) == 0) h->rcode = 3;
    }

    cout << "response length: " << res_len << endl;

    sendto(fd, buf, res_len, 0, (sockaddr *) & remaddr, sizeof(remaddr));
  }

  return 0;
}
