#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

#include "dns.h"

// ### get_names
// Creates a map of dns protocol formatted names to ips from a file
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
    size_t pound = ip.find_first_of("#");
    if (pound != string::npos) {
      ip.erase(pound, pound - ip.length());
    }

    output[name_to_dns(domain)] = ip;
  }

  return output;
}
