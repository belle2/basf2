#include "tracking/modules/CATFinder/KDTNodePool.h"
#include <framework/logging/Logger.h>


KDTNodePool::KDTNodePool(size_t capacity) : index(0)
{
  pool.resize(capacity);
  for (size_t i = 0; i < capacity; ++i) {
    pool[i] = new KDTNode();
  }
}

KDTNodePool::~KDTNodePool()
{
  for (KDTNode* node : pool) {
    delete node;
  }
}

KDTNode* KDTNodePool::allocate()
{
  if (index >= pool.size()) {
    B2ERROR("KDTNodePool:allocate() exceeded pool capacity.");
  }
  return pool[index++];
}

void KDTNodePool::reset()
{
  index = 0;
}
