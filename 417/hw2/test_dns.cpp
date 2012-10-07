#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

#include "dns.h"

int
main (int argc, char ** argv) {
  auto ip = "www.google.com";
  auto dns_repr = name_to_dns(ip);

  assert(dns_repr[0] == 3);
  assert(dns_repr[1] == 'w');

  assert(dns_repr[4] == 6);
  assert(dns_repr[7] == 'o');

  assert(dns_repr[15] == 0);
}
