#include "parser_input.h"
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[]) {
  assert(argc == 2);

  std::string path(argv[1]);
  Partition::readDataFromFile(path);

  return 0;
}
