
#include "coarsening.h"
#include "definition.h"
#include "fm_partition.h"
#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <ctime>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <utility>
using namespace Partition;

Index randomNumberGenerator(Index lower, Index upper) {
  static std::default_random_engine rng(time(0));

  std::uniform_int_distribution<Index> dist(lower, upper);
  return dist(rng);
}

std::map<Index, int> Partition::Multilevel(HyperGraph &graph, float ratio,
                                           size_t minimum_size) {
  assert(ratio > 0 && ratio < 1);
  std::set<Index> part_1;
  std::set<Index> part_2;
  auto getEdgeByOrderWeight = [&graph]() {
    std::vector<Index> vec(graph.bitMatrix.size());
    for (auto i = 0; i < vec.size(); i++) {
      vec[i] = i;
    }
    std::sort(vec.begin(), vec.end(), [&graph](Index a, Index b) {
      return (1 / (graph.bitMatrix[a].numberOfOnes() +
                   graph.weight_of_edges->at(a) - 1)) <
             (1 / (graph.bitMatrix[b].numberOfOnes() +
                   graph.weight_of_edges->at(b) - 1));
    });
    return vec;
  };

  if (graph.weight_of_nodes->size() > minimum_size) {
    std::vector<std::vector<Index>> node_to_nodes_map;
    std::vector<bitmap> node_to_nodes_map_bit;
    std::map<size_t, std::pair<bitmap, std::vector<Index>>> edge_to_nodes_map;

    std::vector<Index> sorted_edge = getEdgeByOrderWeight();
    std::set<Index> node_used_checker;
    size_t big_node = 0;
    for (auto iter = sorted_edge.begin(); iter != sorted_edge.end(); iter++) {
      if (node_to_nodes_map.size() == big_node) {
        node_to_nodes_map.push_back(std::vector<Index>());
      }

      std::vector<Index> nodes = graph.bitMatrix[*iter].toVector();
      for (auto n : nodes) {
        if (node_used_checker.find(n) == node_used_checker.end()) {
          node_used_checker.insert(n);
          node_to_nodes_map[big_node].push_back(n);
          if (node_to_nodes_map[big_node].size() >= minimum_size) {
            big_node++;
            /* just break, I will collect the nodes not used at next stage */
            break;
          }
        }
      }
    }

    bitmap node_selected;
    bitmap node_mask;
    for (auto iter = node_used_checker.begin(); iter != node_used_checker.end();
         iter++) {
      node_selected.set(*iter);
    }

    for (auto i = 0; i < graph.weight_of_nodes->size(); i++) {
      node_mask.set(i);
    }

    std::vector<Index> nodes_left = (node_mask - node_selected).toVector();
    for (auto n : nodes_left) {
      node_to_nodes_map.push_back(std::vector<Index>({n}));
    }

    /* bad code, need to optimized, but for time */
    /* I hit to write slow programs but I have to */
    for (auto i = 0; i < node_to_nodes_map.size(); i++) {
      std::sort(node_to_nodes_map[i].begin(), node_to_nodes_map[i].end());
      bitmap temp;
      for (auto n : node_to_nodes_map[i]) {
        temp.set(n);
      }
      node_to_nodes_map_bit.push_back(temp);
    }

    /* another bad code */
    std::map<int, int> weight_of_edges;
    auto hash = [](size_t base, size_t value) {
      static std::hash<size_t> hasher;
      return base * 24 + hasher(value);
    };

    for (auto e : graph.bitMatrix) {
      std::vector<Index> nodes;
      size_t hash_value = 0;
      bitmap mask;
      for (auto i = 0; i < node_to_nodes_map_bit.size(); i++) {
        if (!(e & node_to_nodes_map_bit[i]).empty()) {
          nodes.push_back(i);
          hash_value = hash(hash_value, i);
          mask.set(i);
        }
      }
      if (edge_to_nodes_map.find(hash_value) != edge_to_nodes_map.end()) {
        weight_of_edges[hash_value] += 1;
      } else {
        weight_of_edges[hash_value] = 1;
        edge_to_nodes_map.insert(
            std::pair<size_t, std::pair<bitmap, std::vector<Index>>>(
                hash_value,
                std::pair<bitmap, std::vector<Index>>(mask, nodes)));
      }
    }

    /* construct a new HyperGraph */
    std::vector<bitmap> result_matrix;
    std::vector<int> result_node_weight;
    std::vector<int> result_edge_weight;
    for (auto iter : edge_to_nodes_map) {
      if (iter.second.second.size() > 1) {
        result_matrix.push_back(iter.second.first);
        result_edge_weight.push_back(weight_of_edges.find(iter.first)->second);
      }
    }

    for (auto iter : node_to_nodes_map) {
      result_node_weight.push_back(iter.size());
    }
    HyperGraph new_graph(result_matrix, result_edge_weight, result_node_weight);

    std::map<Index, int> result = Multilevel(new_graph, ratio, minimum_size);

    for (auto n : result) {
      std::vector<Index> nodes = node_to_nodes_map[n.first];
      if (n.second == 1) {
        part_1.insert(nodes.begin(), nodes.end());
      } else if (n.second == 2) {
        part_2.insert(nodes.begin(), nodes.end());
      }
    }
    FM(part_1, part_2, graph, ratio, 2);
  } else {
    /* initial partitioning stage */

    std::vector<Index> unique_counter;
    std::vector<Index> sorted_edge = getEdgeByOrderWeight();
    std::set<Index> node_used_checker;
    for (auto iter = sorted_edge.begin(); iter != sorted_edge.end(); iter++) {
      std::vector<Index> nodes = graph.bitMatrix[*iter].toVector();
      for (auto n : nodes) {
        if (node_used_checker.find(n) == node_used_checker.end()) {
          node_used_checker.insert(n);
          unique_counter.push_back(n);
        }
      }
    }

    size_t total_area = 0;
    for (auto i = 0; i < graph.weight_of_nodes->size(); i++) {
      total_area += graph.weight_of_nodes->at(i);
    }

    size_t part_1_area = 0;
    size_t part_2_area = 0;

    for (auto iter = unique_counter.begin(); iter != unique_counter.end();
         iter++) {
      if (part_1_area < total_area * ratio) {
        part_1.insert(*iter);
        part_1_area += graph.weight_of_nodes->at(*iter);
      } else {
        part_2.insert(*iter);
        part_2_area += graph.weight_of_nodes->at(*iter);
      }
    }

    FM(part_1, part_2, graph, ratio, 10);
  }

  size_t total_cut = 0;
  bitmap part_1_map;
  bitmap part_2_map;
  std::map<Index, int> result;
  for (auto n : part_1) {
    result.insert(std::pair<Index, int>(n, 1));
    part_1_map.set(n);
  }
  for (auto n : part_2) {
    result.insert(std::pair<Index, int>(n, 2));
    part_2_map.set(n);
  }

  for (auto e : graph.bitMatrix) {
    if (!e.logicaland(part_1_map).empty() &&
        !e.logicaland(part_2_map).empty()) {
      total_cut++;
    }
  }
  std::cout << "total_cut: " << total_cut << std::endl;

  return result;
}
