/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef SIMULATION_DATAOBJECTS_MCTRAJECTORYPOINT_H
#define SIMULATION_DATAOBJECTS_MCTRAJECTORYPOINT_H

namespace Belle2 {
  /** Small struct to encode a position/momentum without additional overhead */
  struct MCTrajectoryPoint {
    /** Constructor to allow initialization */
    MCTrajectoryPoint(float x = 0, float y = 0, float z = 0, float px = 0, float py = 0, float pz = 0):
      x(x), y(y), z(z), px(px), py(py), pz(pz) {}
    float x;  /**< x coordinate */
    float y;  /**< y coordinate */
    float z;  /**< z coordinate */
    float px; /**< momentum along x */
    float py; /**< momentum along y */
    float pz; /**< momentum along z */
  };
} //Belle2 namespace

#endif // SIMULATION_DATAOBJECTS_TRAJECTORYPOINT_H
