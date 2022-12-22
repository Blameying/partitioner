#pragma once

#include "definition.h"
namespace Partition {

class Multilevel {
private:
  float ratio = 0;
  size_t minimal_size = 0;

public:
  Multilevel(HyperGraph &graph, float ratio, size_t minimal_size) {}
};
}; // namespace Partition
