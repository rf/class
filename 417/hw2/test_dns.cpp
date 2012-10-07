#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

#include "dns.h"

char req[] = {
static_cast<char> (0xa5), static_cast<char> (0xa7), 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x04, 0x74, 0x65, 0x73, 
0x74, 0x03, 0x61, 0x62, 0x63, 0x01, 0x64, 0x00, 
0x00, 0x01, 0x00, 0x01 };

int
main (int argc, char ** argv) {
  auto ip = "www.google.com";
  auto dns_repr = name_to_dns(ip);

  assert(dns_repr[0] == 3);
  assert(dns_repr[1] == 'w');

  assert(dns_repr[4] == 6);
  assert(dns_repr[7] == 'o');

  assert(dns_repr[15] == 0);

  int q_len;
  try {
    auto parsed = parse_dns_req(req, &q_len);
  } catch (char const * e) {
    cout << e << endl;
    return 1;
  }

  auto intip = strip_to_intip("13.4.255.39");
  unsigned char * p = (unsigned char *) & intip;
  assert(p[0] == 13);
  assert(p[1] == 4);
  assert(p[2] == 255);
  assert(p[3] == 39);
}
