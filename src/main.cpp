#include "coarsening.h"
#include "definition.h"
#include "fm_partition.h"
#include "parser_input.h"
#include <assert.h>
#include <cstddef>
#include <iostream>
#include <map>

using namespace Partition;
int main(int argc, char *argv[]) {
  assert(argc == 2);

  std::string path(argv[1]);
  HyperGraph graph = readDataFromFile(path);
  Multilevel(graph, 0.5, 8);

  return 0;
}
