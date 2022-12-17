#include "fm_partition.h"
#include "definition.h"

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

FM::FM(std::vector<Index> &part_1, std::vector<Index> &part_2,
       HyperGraph &graph, float ratio)
    : ratio(ratio) {
  std::map<Index, float> initial_gain;
  std::vector<Index> all_nodes = part_1;
  all_nodes.insert(all_nodes.end(), part_2.begin(), part_2.end());

  for (auto n : part_1) {
    float gain = 0;
    for
      i
  }
}

} // namespace Partition
