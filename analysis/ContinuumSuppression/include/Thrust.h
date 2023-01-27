/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <Math/Vector3D.h>

#include <vector>


namespace Belle2 {

  /** Class to calculate the thrust axis */
  class Thrust {
  public:
    /**
     * calculates the thrust axis
     */
    static ROOT::Math::XYZVector calculateThrust(const std::vector<ROOT::Math::XYZVector>& momenta);
  };


} // Belle2 namespace


