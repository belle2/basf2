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
    MCTrajectoryPoint(float x_ = 0, float y_ = 0, float z_ = 0, float px_ = 0, float py_ = 0, float pz_ = 0):
      x(x_), y(y_), z(z_), px(px_), py(py_), pz(pz_) {}
    float x;  /**< x coordinate */
    float y;  /**< y coordinate */
    float z;  /**< z coordinate */
    float px; /**< momentum along x */
    float py; /**< momentum along y */
    float pz; /**< momentum along z */
  };
} //Belle2 namespace

#endif // SIMULATION_DATAOBJECTS_TRAJECTORYPOINT_H
