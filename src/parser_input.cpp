#include "parser_input.h"
#include "definition.h"
#include <assert.h>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

Partition::HyperGraph Partition::readDataFromFile(std::string path) {
  std::ifstream input;
  input.open(path);

  assert(input.is_open());

  std::string line;
  std::getline(input, line);
  std::istringstream stream(line);

  Index edges_count = 0;
  Index nodes_count = 0;

  stream >> edges_count >> nodes_count;

  std::vector<Index> edges;
  std::vector<Index> nodes;

  for (auto i = 0; i < edges_count; i++) {
    std::string line;
    std::getline(input, line);
    if (line.empty()) {
      break;
    }
    std::istringstream stream(line);

    Index edge_id = 0;
    /* edge id will not be used */
    stream >> edge_id;

    edges.push_back(nodes.size());
    Index pin = 0;
    while (stream >> pin) {
      assert(pin <= nodes_count);
      nodes.push_back(pin - 1);
    }
  }

  /* value in edges is the index of the start node of the nodes belonged to this
   * edge in nodes*/
  return HyperGraph(edges, nodes);
}
