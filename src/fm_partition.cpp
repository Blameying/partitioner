#include "fm_partition.h"
#include "definition.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
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

FM::FM(std::set<Index> &part_1, std::set<Index> &part_2,
       std::map<Index, int> &areas, HyperGraph &graph, float ratio)
    : ratio(ratio) {
  std::vector<Index> all_nodes;

  /* area related value */
  size_t total_area = 0;
  size_t max_area = 0;
  size_t part_1_area = 0;
  size_t part_2_area = 0;
  for (auto i : areas) {
    all_nodes.push_back(i.first);
    total_area += i.second;
    if (max_area < i.second) {
      max_area = i.second;
    }
    if (part_1.find(i.first) != part_1.end()) {
      part_1_area += i.second;
    } else {
      part_2_area += i.second;
    }
  }

  std::map<Index, bitmap> matrix = graph.getEdgesBitmapAmongNodes(all_nodes);

  auto condition_checker = [ratio, total_area, max_area](size_t area) -> bool {
    /* Assume that the area of all nodes is 1 */
    return (area >= ratio * total_area - max_area) &&
           (area <= ratio * total_area + max_area);
  };

  auto highest_gain = [&part_1, condition_checker, &part_1_area, this,
                       &areas](Index index) -> bool {
    if (this->locked.find(index) != this->locked.end()) {
      return false;
    }

    size_t index_area = areas.find(index)->second;
    if (part_1.find(index) != part_1.end()) {
      return condition_checker(part_1_area - index_area);
    } else {
      return condition_checker(part_1_area + index_area);
    }
  };

  initBucketSorter(part_1, part_2, matrix);
  while (true) {
    Index need_to_move = 0;
    if (sorter->getHighAvalible(need_to_move, highest_gain)) {
      /* update area infomation */
      size_t need_to_move_area = areas.find(need_to_move)->second;
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
    for (auto iter = matrix.begin(); iter != matrix.end(); iter++) {
      if (iter->second.get(need_to_move)) {
        bitmap and_1 = part_1_map.logicaland(iter->second);
        bitmap and_2 = part_2_map.logicaland(iter->second);

        if (and_1.empty()) {
          for (auto v : and_2.toArray()) {
            sorter->incrementGain(v, -2);
          }
        } else if (and_2.empty()) {
          for (auto v : and_1.toArray()) {
            sorter->incrementGain(v, -2);
          }
        } else {
          if (and_1.get(need_to_move) && and_2.numberOfOnes() == 1) {
            int change = 0;
            if (iter->second.numberOfOnes() > 2) {
              change = 1;
            } else {
              change = 2;
            }

            for (auto v : and_1.toArray()) {
              sorter->incrementGain(v, -change);
            }
            for (auto v : and_2.toArray()) {
              sorter->incrementGain(v, change);
            }
          } else if (and_2.get(need_to_move) && and_1.numberOfOnes() == 1) {
            int change = 0;
            if (iter->second.numberOfOnes() > 2) {
              change = 1;
            } else {
              change = 2;
            }

            for (auto v : and_1.toArray()) {
              sorter->incrementGain(v, change);
            }
            for (auto v : and_2.toArray()) {
              sorter->incrementGain(v, -change);
            }
          }
        }
      }
    }
  }
}

void FM::initBucketSorter(std::set<Index> &part_1, std::set<Index> &part_2,
                          std::map<Index, bitmap> &matrix) {
  sorter = new BucketSorter(-matrix.size(), matrix.size());
  bitmap part_1_map;
  bitmap part_2_map;
  for (auto n : part_1) {
    part_1_map.set(n);
  }

  for (auto n : part_2) {
    part_2_map.set(n);
  }

  for (auto iter = matrix.begin(); iter != matrix.end(); iter++) {
    auto and_1 = iter->second.logicaland(part_1_map);
    auto and_2 = iter->second.logicaland(part_2_map);
    size_t count_1 = and_1.numberOfOnes();
    size_t count_2 = and_2.numberOfOnes();

    if (count_1 == 0) {
      /* internal connection in part_2 */
      for (auto n : and_2.toArray()) {
        sorter->incrementGain(n, -1);
      }
    } else if (count_2 == 0) {
      for (auto n : and_1.toArray()) {
        sorter->incrementGain(n, -1);
      }
    } else {
      if (count_1 == 1) {
        /* perfect external connection */
        for (auto n : and_1.toArray()) {
          sorter->incrementGain(n, 1);
        }
      }
      if (count_2 == 1) {
        for (auto n : and_2.toArray()) {
          sorter->incrementGain(n, 1);
        }
      }
    }
#if DEBUG
    sorter->debugInfo();
#endif
  }
}

} // namespace Partition
