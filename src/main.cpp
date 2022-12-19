#include "definition.h"
#include "fm_partition.h"
#include "parser_input.h"
#include <assert.h>
#include <iostream>

using namespace Partition;
int main(int argc, char *argv[]) {
  assert(argc == 2);

  std::string path(argv[1]);
  HyperGraph graph = readDataFromFile(path);

  std::set<Index> first({1, 2});
  std::set<Index> second({3, 4, 5});
  FM fm = FM(first, second, graph, 0.5);

  std::cout << "Part 1: " << std::endl;
  for (auto i : first) {
    std::cout << i << " " << std::endl;
  }
  std::cout << std::endl << "Part 2: " << std::endl;
  for (auto i : second) {
    std::cout << i << " " << std::endl;
  }

  return 0;
}
