/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/**
 * @struct KDTHit
 * @brief Lightweight 2D representation of a CDC hit for KD-tree sorting.
 *
 * Represents a CDC hit by its 2D coordinates and a corresponding index.
 */
typedef struct KDTHit {
  /** X coordinate of the hit. */
  double x;
  /** Y coordinate of the hit. */
  double y;
  /** Index of the hit in the original input collection. */
  int hitIndex;
  /**
   * @brief Compute squared distance to another KDTHit.
   * @param other Another KDTHit.
   * @return Squared Euclidean distance.
   */
  double squaredDistanceTo(const KDTHit& other) const
  {
    double dx = x - other.x;
    double dy = y - other.y;
    return dx * dx + dy * dy;
  }
} KDTHit;