#pragma once

#include <cstddef>
#include "KDTNode.h"
#include <vector>


class KDTNodePool {
private:
  std::vector<KDTNode*> pool;
  size_t index;

public:
  KDTNodePool(size_t capacity);
  ~KDTNodePool();

  KDTNodePool(const KDTNodePool&) = delete;
  KDTNodePool& operator=(const KDTNodePool&) = delete;

  KDTNode* allocate();
  void reset();

};
