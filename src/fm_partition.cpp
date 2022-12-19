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
       float ratio)
    : ratio(ratio) {
  std::vector<Index> all_nodes(part_1.begin(), part_1.end());
  all_nodes.insert(all_nodes.end(), part_2.begin(), part_2.end());
  std::map<Index, bitmap> matrix = graph.getEdgesBitmapAmongNodes(all_nodes);
  sorter = new BucketSorter(-matrix.size(), matrix.size());

  while (true) {
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
    break;
  }
}

} // namespace Partition
