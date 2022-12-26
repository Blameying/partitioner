#include "coarsening.h"
#include "definition.h"
#include "fm_partition.h"
#include "parser_input.h"
#include <assert.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>

using namespace Partition;
int main(int argc, char *argv[]) {
  assert(argc == 3);

  float ratio = atof(argv[1]);
  std::string path(argv[2]);
  HyperGraph graph = readDataFromFile(path);
  std::map<Index, int> result = Multilevel(graph, ratio, 8);

  std::ofstream output_file;
  std::ostringstream buffer;
  buffer << "output_" << graph.weight_of_nodes->size() << ".txt";
  output_file.open(buffer.str());
  for (auto i = result.begin(); i != result.end(); i++) {
    output_file << i->first << " " << i->second - 1 << std::endl;
  }
  output_file.close();

  return 0;
}
