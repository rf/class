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

int
main (int argc, char ** argv) {
  auto names = get_names("test.txt");
  int rc;

  char * strport = getenv("DNS_PORT");
  int port;
  if (!strport) port = 20000;
  else port = atoi(strport);

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

    auto parsed = parse_dns_req(buf, &q_len);
    res_len = 0;

    char * ptr = buf;

    dns_header * h = (dns_header *) ptr;
    h->qr = 1;
    res_len += sizeof(dns_header) + q_len;

    if (parsed && names.count(*parsed) == 1) {
      cout << "Returning: " << names[*parsed] << endl;

      h->aa = 1;
      h->tc = 0;
      h->ra = 0;
      h->rcode = 0;
      h->an_count = htons(1);

      ptr += sizeof(dns_header) + q_len;

      memcpy(ptr, parsed->c_str(), strlen(parsed->c_str()) + 1);
      ptr += strlen(parsed->c_str()) + 1;
      res_len += strlen(parsed->c_str()) + 1;

      dns_ip_answer * a = (dns_ip_answer *) ptr;
      memset(a, 0, sizeof(dns_ip_answer));
      a->type = htons(1);
      a->klass = htons(1);;
      a->data_len = htons(4);
      a->ttl = 0;
      a->address = (strip_to_intip(names[*parsed]));

      res_len += sizeof(dns_ip_answer);
    }

    cout << "response length: " << res_len << endl;

    sendto(fd, buf, res_len, 0, (sockaddr *) & remaddr, sizeof(remaddr));
  }

  return 0;
}
