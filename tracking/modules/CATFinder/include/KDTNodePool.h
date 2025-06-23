/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <cstddef>
#include <tracking/modules/CATFinder/KDTNode.h>
#include <vector>

/**
 * @class KDTNodePool
 * @brief Memory pool for pre-allocating and reusing KDTNode objects.
 *
 * Improves performance by avoiding dynamic memory allocation during KD-tree construction.
 * Nodes are allocated once at initialization and reset between KD-tree builds.
 */
class KDTNodePool {
private:
  /** Internal storage for preallocated KD-tree nodes. */
  std::vector<KDTNode*> pool;
  /** Current index for the next available node in the pool. */
  size_t index;

public:
  /**
   * @brief Constructs a pool with the specified capacity.
   *
   * Preallocates memory for a given number of KD-tree nodes.
   *
   * @param capacity Maximum number of KDTNodes to allocate.
   */
  KDTNodePool(size_t capacity);

  /**
   * @brief Destructor. Frees all allocated KDTNode objects.
   */
  ~KDTNodePool();

  KDTNodePool(const KDTNodePool&) = delete;
  KDTNodePool& operator=(const KDTNodePool&) = delete;

  /**
   * @brief Allocates and returns the next available KDTNode from the pool.
   *
   * @return Pointer to a KDTNode.
   * @note Logs an error if the pool capacity is exceeded.
   */
  KDTNode* allocate();

  /**
   * @brief Resets the pool index, making all nodes available for reuse.
   *
   * Does not deallocate any memory.
   */
  void reset();

};
