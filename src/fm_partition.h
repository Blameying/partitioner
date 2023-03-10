#include "definition.h"
#include <assert.h>
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <vector>

namespace Partition {

class BucketSorter {
private:
  int _low = 0;
  int _high = 0;
  int _max = 0;
  int _min = 0;
  std::vector<std::list<Index> *> *bucket = nullptr;
  std::map<Index, int> _idSet;

public:
  BucketSorter(int low, int high) : _low(low), _high(high) {
    bucket = new std::vector<std::list<Index> *>();
    for (int i = low; i <= high; i++) {
      std::list<Index> *list = new std::list<Index>();
      bucket->push_back(list);
    }
    _max = low;
    _min = high;
    _idSet.clear();
  };

  ~BucketSorter() {
    for (auto iter = bucket->begin(); iter != bucket->end(); iter++) {
      delete *iter;
    }
    delete bucket;
  }

  bool addValue(Index id, int gain);
  bool updateValue(Index id, int gain);
  void removeValue(Index id);
  void removeValueWithGain(Index id, int gain);
  bool getMax(Index &index);
  bool getMin(Index &index);
  int getGain(Index &index);
  int incrementGain(Index &index, int value);
  int incrementExistGain(Index &index, int value);
  bool getHighAvalible(Index &index, std::function<bool(Index)> filter);
  int getAllGain();
  void debugInfo();
};

class FM {
private:
  float ratio = 0.0;
  BucketSorter *sorter = nullptr;
  std::set<Index> locked;

private:
  void initBucketSorter(std::set<Index> &part_1, std::set<Index> &part_2,
                        HyperGraph &graph);

public:
  FM(std::set<Index> &part_1, std::set<Index> &part_2, HyperGraph &graph,
     float ratio) {
    FM(part_1, part_2, graph, ratio, 0);
  };
  FM(std::set<Index> &part_1, std::set<Index> &part_2, HyperGraph &graph,
     float ratio, int k);
  ~FM() { delete sorter; };
};

}; // namespace Partition
