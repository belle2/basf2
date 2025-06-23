/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/modules/CATFinder/KDTHit.h>
#include <cstddef>

/**
 * @struct KDTNode
 * @brief Node structure for a 2D KD-tree used in spatial hit sorting.
 *
 * Represents a node in a binary KD-tree with left and right child pointers,
 * and metadata for axis splitting and usage tracking.
 */
typedef struct KDTNode {
  /** The hit associated with this node (contains x, y, and index). */
  KDTHit hit;
  /** Pointer to the left subtree (for values less than split). */
  KDTNode* left;
  /** Pointer to the right subtree (for values greater than split). */
  KDTNode* right;
  /** Flag indicating whether the node has already been used in traversal. */
  bool used;
  /** Splitting dimension: 0 for x-axis, 1 for y-axis. */
  int dim;
} KDTNode;