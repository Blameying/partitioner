#include "fm_partition.h"
#include "definition.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <utility>

namespace Partition {

bool BucketSorter::addValue(Index id, int gain) {
  assert(gain >= _low && gain <= _high);
  if (_idSet.find(id) != _idSet.end()) {
    return false;
  }

  _idSet.insert(std::pair<Index, int>(id, gain));
  (*bucket)[gain - _low]->push_back(id);
  if (gain > _max) {
    _max = gain;
  }

  if (gain < _min) {
    _min = gain;
  }

  return true;
}

bool BucketSorter::updateValue(Index id, int gain) {
  assert(gain >= _low && gain <= _high);

  auto iter = _idSet.find(id);
  if (iter != _idSet.end()) {
    auto link_root = (*bucket)[iter->second - _low];
    link_root->remove(id);
    if (iter->second == _max) {
      for (int i = _max; i > _min; i--) {
        if ((*bucket)[i - _low]->size() != 0) {
          _max = i;
          break;
        }
      }
    } else if (iter->second == _min) {
      for (int i = _min; i < _max; i++) {
        if ((*bucket)[i - _low]->size() != 0) {
          _min = i;
          break;
        }
      }
    }
    /* update */
    iter->second = gain;
  } else {
    _idSet.insert(std::pair<Index, int>(id, gain));
  }

  (*bucket)[gain - _low]->push_back(id);
  if (gain > _max) {
    _max = gain;
  }

  if (gain < _min) {
    _min = gain;
  }

  return true;
}

void BucketSorter::removeValue(Index id) {
  auto iter = _idSet.find(id);
  if (iter != _idSet.end()) {
    (*bucket)[iter->second - _low]->remove(id);

    if (iter->second == _max) {
      for (int i = _max; i > _min; i--) {
        if ((*bucket)[i - _low]->size() != 0) {
          _max = i;
          break;
        }
      }
    } else if (iter->second == _min) {
      for (int i = _min; i < _max; i++) {
        if ((*bucket)[i - _low]->size() != 0) {
          _min = i;
          break;
        }
      }
    }
    _idSet.erase(iter);
  }
}

void BucketSorter::removeValueWithGain(Index id, int gain) {
  assert(gain >= _low && gain <= _high);

  bucket->at(gain - _low)->remove(id);
  _idSet.erase(id);
  if (gain == _max) {
    for (int i = _max; i > _min; i--) {
      if ((*bucket)[i - _low]->size() != 0) {
        _max = i;
        break;
      }
    }
  } else if (gain == _min) {
    for (int i = _min; i < _max; i++) {
      if ((*bucket)[i - _low]->size() != 0) {
        _min = i;
        break;
      }
    }
  }
}

bool BucketSorter::getMax(Index &index) {
  if (bucket->at(_max - _low)->size() != 0) {
    index = bucket->at(_max - _low)->front();
    return true;
  }

  return false;
}

bool BucketSorter::getMin(Index &index) {
  if (bucket->at(_min - _low)->size() != 0) {
    index = bucket->at(_min - _low)->front();
    return true;
  }

  return false;
}

int BucketSorter::getGain(Index &index) {
  auto iter = _idSet.find(index);
  if (iter != _idSet.end()) {
    return iter->second;
  }
  return 0;
}

int BucketSorter::incrementGain(Index &index, int value) {
  int gain = 0;
  auto iter = _idSet.find(index);
  if (iter != _idSet.end()) {
    gain = iter->second;
  }

  gain += value;

  updateValue(index, gain);
  return gain;
}

int BucketSorter::incrementExistGain(Index &index, int value) {
  int gain = 0;
  auto iter = _idSet.find(index);
  if (iter != _idSet.end()) {
    gain = iter->second;
  } else {
    return 0;
  }

  gain += value;

  updateValue(index, gain);
  return gain;
}

bool BucketSorter::getHighAvalible(Index &index,
                                   std::function<bool(Index)> filter) {
  if (_idSet.size() == 0) {
    return false;
  }

  for (int i = _max - _low; i > _min - _low; i--) {
    for (auto iter = (*bucket)[i]->begin(); iter != (*bucket)[i]->end();
         iter++) {
      if (filter(*iter)) {
        index = *iter;
        return true;
      }
    }
  }
  return false;
}

void BucketSorter::debugInfo() {
  std::cout << "low: " << _low << " high: " << _high << std::endl;
  std::cout << "Gain range: " << _min << "::" << _max << std::endl;
  std::cout << "{ ";
  for (auto iter : _idSet) {
    std::cout << iter.first << ":" << iter.second << ", ";
  }
  std::cout << " }" << std::endl;

  for (auto iter = _min - _low; iter <= _max - _low; iter++) {
    std::cout << "Gain " << iter + _low << ": size " << (*bucket)[iter]->size()
              << std::endl;
  }
}

FM::FM(std::set<Index> &part_1, std::set<Index> &part_2, HyperGraph &graph,
       float ratio, int k)
    : ratio(ratio) {
  graph.debugInfo();
  /* ignore the first value, and it belongs to no node */
  auto total_area = std::accumulate(graph.weight_of_nodes->begin() + 1,
                                    graph.weight_of_nodes->end(), 0);
  auto max_area = std::max_element(graph.weight_of_nodes->begin() + 1,
                                   graph.weight_of_nodes->end());
  size_t part_1_area = 0;
  size_t part_2_area = 0;
  for (auto i : part_1) {
    part_1_area += graph.weight_of_nodes->at(i);
  }
  for (auto i : part_2) {
    part_2_area += graph.weight_of_nodes->at(i);
  }

  /* limitation of the sizes of two parts */
  auto condition_checker = [ratio, total_area, max_area](size_t area) -> bool {
    return (area >= ratio * total_area - *max_area) &&
           (area <= ratio * total_area + *max_area);
  };

  auto highest_gain = [&part_1, condition_checker, &part_1_area, this,
                       &graph](Index index) -> bool {
    if (this->locked.find(index) != this->locked.end()) {
      return false;
    }

    size_t index_area = graph.weight_of_nodes->at(index);
    if (part_1.find(index) != part_1.end()) {
      return condition_checker(part_1_area - index_area);
    } else {
      return condition_checker(part_1_area + index_area);
    }
  };

  initBucketSorter(part_1, part_2, graph);

  while (true) {
    Index need_to_move = 0;
    if (sorter->getHighAvalible(need_to_move, highest_gain)) {
      /* update area infomation */
      size_t need_to_move_area = graph.weight_of_nodes->at(need_to_move);
      if (part_1.find(need_to_move) != part_1.end()) {
        part_2.insert(need_to_move);
        part_1.erase(need_to_move);
        part_1_area -= need_to_move_area;
        part_2_area += need_to_move_area;
      } else {
        part_1.insert(need_to_move);
        part_2.erase(need_to_move);
        part_1_area += need_to_move_area;
        part_2_area -= need_to_move_area;
      }
      locked.insert(need_to_move);
      sorter->removeValue(need_to_move);

      if (k != 0 && locked.size() == k) {
        break;
      }
    } else {
      break;
    }

    bitmap part_1_map;
    bitmap part_2_map;

    for (auto n : part_1) {
      part_1_map.set(n);
    }
    for (auto n : part_2) {
      part_2_map.set(n);
    }
    for (auto iter = graph.bitMatrix.begin(); iter != graph.bitMatrix.end();
         iter++) {
      if (iter->get(need_to_move)) {
        bitmap and_1 = part_1_map.logicaland(*iter);
        bitmap and_2 = part_2_map.logicaland(*iter);

        std::cout << *iter << std::endl;
        size_t edge_weight =
            graph.weight_of_edges->at(iter - graph.bitMatrix.begin());

        if (and_1.empty()) {
          for (auto v : and_2.toArray()) {
            if (v != need_to_move) {
              sorter->incrementExistGain(v, -2 * edge_weight);
            }
          }
        } else if (and_2.empty()) {
          for (auto v : and_1.toArray()) {
            if (v != need_to_move) {
              sorter->incrementExistGain(v, -2 * edge_weight);
            }
          }
        } else {
          if (and_1.get(need_to_move) && and_2.numberOfOnes() == 1) {
            int change = 0;
            if (iter->numberOfOnes() > 2) {
              change = 1 * edge_weight;
            } else {
              change = 2 * edge_weight;
            }

            for (auto v : and_1.toArray()) {
              if (v != need_to_move) {
                sorter->incrementExistGain(v, -change);
              }
            }
            for (auto v : and_2.toArray()) {
              sorter->incrementExistGain(v, change);
            }
          } else if (and_2.get(need_to_move) && and_1.numberOfOnes() == 1) {
            int change = 0;
            if (iter->numberOfOnes() > 2) {
              change = edge_weight;
            } else {
              change = 2 * edge_weight;
            }

            for (auto v : and_1.toArray()) {
              sorter->incrementExistGain(v, change);
            }
            for (auto v : and_2.toArray()) {
              if (v != need_to_move) {
                sorter->incrementExistGain(v, -change);
              }
            }
          }
        }
      }
    }
#if DEBUG
    sorter->debugInfo();
#endif
  }
}

void FM::initBucketSorter(std::set<Index> &part_1, std::set<Index> &part_2,
                          HyperGraph &graph) {

  /* here we can use a more memory saving method, but not necessary for the
   * given test set */
  size_t range = std::accumulate(graph.weight_of_edges->begin(),
                                 graph.weight_of_edges->end(), 0);
  sorter = new BucketSorter(-range, range);
  bitmap part_1_map;
  bitmap part_2_map;
  for (auto n : part_1) {
    part_1_map.set(n);
  }

  for (auto n : part_2) {
    part_2_map.set(n);
  }

  for (auto iter = graph.bitMatrix.begin(); iter != graph.bitMatrix.end();
       iter++) {
    auto and_1 = iter->logicaland(part_1_map);
    auto and_2 = iter->logicaland(part_2_map);
    size_t count_1 = and_1.numberOfOnes();
    size_t count_2 = and_2.numberOfOnes();

    size_t edge_weight =
        graph.weight_of_edges->at(iter - graph.bitMatrix.begin());

    if (count_1 == 0) {
      /* internal connection in part_2 */
      for (auto n : and_2.toArray()) {
        sorter->incrementGain(n, -edge_weight);
      }
    } else if (count_2 == 0) {
      for (auto n : and_1.toArray()) {
        sorter->incrementGain(n, -edge_weight);
      }
    } else {
      if (count_1 == 1) {
        /* perfect external connection */
        for (auto n : and_1.toArray()) {
          sorter->incrementGain(n, edge_weight);
        }
      }
      if (count_2 == 1) {
        for (auto n : and_2.toArray()) {
          sorter->incrementGain(n, edge_weight);
        }
      }
    }
#if DEBUG
    sorter->debugInfo();
#endif
  }
}

} // namespace Partition
