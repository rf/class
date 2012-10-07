#include <string>
#include <sstream>
#include <cstring>

using namespace std;

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
