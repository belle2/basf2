/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TVector3.h>

#include <vector>


namespace Belle2 {

  /** Class to calculate the thrust axis */
  class Thrust {
  public:
    /**
     * calculates the thrust axis
     */
    static TVector3 calculateThrust(const std::vector<TVector3>& momenta);
  };


} // Belle2 namespace


