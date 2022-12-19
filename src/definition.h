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

class Node;

class Edge {
private:
  Index id;
  std::set<Index> nodes;

public:
  Edge(Index id) : id(id){};
  void addNode(Index node) { nodes.insert(node); };
  Index getId() { return id; };
  std::set<Index> getNodes() { return nodes; };
};

class Node {
private:
  Index id;
  std::set<Index> edges;

public:
  Node(Index id) : id(id){};
  void addEdge(Index edge) { edges.insert(edge); };
  Index getId() { return id; };
  std::set<Index> getEdges() { return edges; };
};

class HyperGraph {
private:
  std::map<Index, Edge> edges;
  std::map<Index, Node> nodes;

  std::vector<bitmap> bitMatrix;

public:
  HyperGraph(std::vector<Index> &edges_index, std::vector<Index> &node_index) {
    assert(edges_index.size() > 0);
    assert(node_index.size() > 0);
    if (edges_index[edges_index.size() - 1] < node_index.size()) {
      edges_index.push_back(node_index.size());
    }

    for (auto i = 0; i < edges_index.size() - 1; i++) {
      auto edge_iter = edges.insert(std::pair<Index, Edge>(i, Edge(i)));
      bitmap bitLine;
      for (auto n = edges_index[i]; n < edges_index[i + 1]; n++) {
        auto node_iter = nodes.insert(
            std::pair<Index, Node>(node_index[n], Node(node_index[n])));
        node_iter.first->second.addEdge(i);
        edge_iter.first->second.addNode(node_index[n]);
        bitLine.set(node_index[n]);
      }
      bitMatrix.push_back(bitLine);
      std::cout << "edge: " << i << "nodes: " << bitLine << std::endl;
    }

    assert(edges.size() == bitMatrix.size());
  }

  std::vector<Index> getEdgesBetweenNodes(Index node_1, Index node_2) {
    assert(node_1 <= nodes.size() && node_2 <= nodes.size());
    std::vector<Index> result;
    for (auto i = 0; i < bitMatrix.size(); i++) {
      if (bitMatrix[i].get(node_1) && bitMatrix[i].get(node_2)) {
        result.push_back(i);
      }
    }
    return result;
  }

  std::vector<Index> getNodesBewteenEdges(Index edge_1, Index edge_2) {
    assert(edge_1 < bitMatrix.size() && edge_2 < bitMatrix.size());

    bitmap andResult = bitMatrix[edge_1] & bitMatrix[edge_2];

    return andResult.toArray();
  }

  bool checkConnectionInEdges(Index node_1, Index node_2,
                              std::vector<Index> &edges) {
    assert(node_1 <= nodes.size() && node_2 <= nodes.size());

    for (auto e : edges) {
      if (e < bitMatrix.size()) {
        if (bitMatrix[e].get(node_1) && bitMatrix[e].get(node_2)) {
          return true;
        }
      }
    }

    return false;
  }

  std::vector<Index> getEdgesAmongNodes(std::vector<Index> &nodes) {
    bitmap comp;
    std::vector<Index> result;

    std::sort(nodes.begin(), nodes.end());
    for (auto n : nodes) {
      comp.set(n);
    }

    for (auto i = 0; i < bitMatrix.size(); i++) {
      if (comp.logicalandcount(bitMatrix[i]) > 1) {
        result.push_back(i);
      }
    }

    return result;
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

  size_t countEdgesWithGroup(Index node, std::vector<Index> &group) {
    auto iter = nodes.find(node);
    if (iter != nodes.end()) {
      auto edges = iter->second.getEdges();
      bitmap map;
      std::sort(group.begin(), group.end());
      for (auto n : group) {
        if (n != node) {
          map.set(n);
        }
      }
      size_t count = 0;
      for (auto e : edges) {
        if (!bitMatrix[e].logicaland(map).empty()) {
          count++;
        }
      }
    }
    return 0;
  }

  bool checkNodeInEdge(Index node, Index edge) {
    assert(edge < bitMatrix.size() && node <= nodes.size());
    return bitMatrix[edge].get(node);
  }

  bool checkNodeInEdgeUnsafe(Index node, Index edge) {
    return bitMatrix[edge].get(node);
  }

  std::vector<Index> getEdges() {
    std::vector<Index> temp;
    for (auto iter = edges.begin(); iter != edges.end(); iter++) {
      temp.push_back(iter->first);
    }
    return temp;
  }

  std::vector<Index> getNodes() {
    std::vector<Index> temp;
    for (auto iter = nodes.begin(); iter != nodes.end(); iter++) {
      temp.push_back(iter->first);
    }
    return temp;
  }
};

}; // namespace Partition
