#include <string>
#include <sstream>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>

using namespace std;

#include "dns.h"

// ### name_to_dns
// Converts an ip to the dns representation of an ip
const char *
name_to_dns (string input) {
  stringstream ss(input);
  char buf[256];
  string output;

  while (ss) {
    ss.getline(buf, 256, '.');
    output.append(1, strlen(buf));
    output.append(buf);
  }

  return output.c_str();
}

// ### parse_dns_req
// Takes an entire dns packet and returns just the string of the requested
// host or null if we can't handle the request
string
parse_dns_req (char * input, int * len, int * type, int * klass) {
  dns_header * h = (dns_header *) input;

  unsigned short num_queries = ntohs(h->qd_count);
  if (num_queries != 1) throw "invalid number of queries";

  char * ptr = input + sizeof(dns_header);
  stringstream ss(ptr);
  char buf[1024];

  ss.getline(buf, 1024, 0);

  ptr += strlen(buf) + 1;

  dns_question * q = (dns_question *) ptr;

  *len = strlen(buf) + 1 + 4;

  *type = ntohs(q->qtype);
  *klass = ntohs(q->qclass);

  return string(buf);
}

// ### strip_to_intip
// Converts an ip in a string to an integer representation of the ip
int
strip_to_intip (string input) {
  int output;
  unsigned char * ptr = (unsigned char *) & output;
  stringstream ss(input);
  char buf[5];

  ss.getline(buf, 5, '.');
  ptr[0] = (unsigned char) atoi(buf);

  ss.getline(buf, 5, '.');
  ptr[1] = (unsigned char) atoi(buf);

  ss.getline(buf, 5, '.');
  ptr[2] = (unsigned char) atoi(buf);

  ss.getline(buf, 5, 0);
  ptr[3] = (unsigned char) atoi(buf);

  return output;
}
