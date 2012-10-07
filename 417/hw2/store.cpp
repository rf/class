#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

#include "dns.h"

const map <string, string>
get_names (string file_name) {
  ifstream file(file_name);
  map <string, string> output;
  char buf[1024];

  while (file) {
    file.getline(buf, 1024);
    stringstream ss(buf);

    if (ss.peek() == '#' || ss.peek() == EOF) continue; 

    string domain, ip;
    ss >> domain >> ip;

    output[name_to_dns(domain)] = ip;
  }

  return output;
}
