#pragma once

#include "definition.h"
namespace Partition {

std::map<Index, int> Multilevel(HyperGraph &graph, float ratio,
                                size_t minimum_size);
}; // namespace Partition
