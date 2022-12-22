#pragma once

#include "ewah/ewah.h"
#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace Partition {

using Index = std::size_t;
using bitmap = ewah::EWAHBoolArray<uint64_t>;

class HyperGraph {
public:
  std::vector<bitmap> bitMatrix;
  std::vector<int> *weight_of_edges;
  std::vector<int> *weight_of_nodes;

public:
  HyperGraph(std::vector<Index> &edges_index, std::vector<Index> &node_index) {
    assert(edges_index.size() > 0);
    assert(node_index.size() > 0);
    /* this function will be called when we parsing the data file */
    /* so all elements have only weight one */
    weight_of_edges = new std::vector<int>();
    auto max = std::max_element(node_index.begin(), node_index.end());
    weight_of_nodes = new std::vector<int>(*max + 1, 1);

    if (edges_index[edges_index.size() - 1] < node_index.size()) {
      edges_index.push_back(node_index.size());
    }

    for (auto i = 0; i < edges_index.size() - 1; i++) {
      bitmap bitLine;
      for (auto n = edges_index[i]; n < edges_index[i + 1]; n++) {
        bitLine.set(node_index[n]);
      }
      bitMatrix.push_back(bitLine);
      weight_of_edges->push_back(1);
#if DEBUG
      std::cout << "edge: " << i << "nodes: " << bitLine << std::endl;
#endif
    }
  }

  HyperGraph(std::vector<bitmap> &graph_info, std::vector<int> &w_edges,
             std::vector<int> &w_nodes) {
    assert(graph_info.size() == w_edges.size());
    bitmap counter;
    for (auto i : graph_info) {
      bitMatrix.push_back(i);
      counter = counter | i;
#if DEBUG
      std::cout << "edge: " << bitMatrix.size() - 1 << "nodes: " << i
                << std::endl;
#endif
    }
    assert(counter.numberOfOnes() == (w_edges.size() + 1));

    weight_of_nodes = new std::vector<int>(w_nodes.begin(), w_nodes.end());
    weight_of_nodes->insert(w_nodes.begin(), 1);
    weight_of_edges = new std::vector<int>(w_edges.begin(), w_edges.end());
  }

  std::map<Index, bitmap> getEdgesBitmapAmongNodes(std::vector<Index> &nodes) {
    bitmap comp;
    std::map<Index, bitmap> result;

    std::sort(nodes.begin(), nodes.end());
    for (auto n : nodes) {
      comp.set(n);
    }

    for (auto i = 0; i < bitMatrix.size(); i++) {
      if (comp.logicalandcount(bitMatrix[i]) > 1) {
        result.insert(std::pair<Index, bitmap>(i, bitMatrix[i]));
      }
    }

    return result;
  }

  ~HyperGraph() {
    weight_of_edges->clear();
    weight_of_edges->shrink_to_fit();
    delete weight_of_edges;
    weight_of_nodes->clear();
    weight_of_nodes->shrink_to_fit();
    delete weight_of_nodes;
  }
};

}; // namespace Partition
