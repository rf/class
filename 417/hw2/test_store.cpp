#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

#include "store.h"

int
main (int argc, char ** argv) {
  auto names = get_names("fixtures/test.txt");

  assert(names.size() == 2);
  assert(names.count("#") == 0);
  assert(names["www.test.org"] == "12.14.101.22");
}
