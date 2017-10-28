/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT)           *
 *   Original module writen by M. Nakao for Belle                         *
 *   Ported to Belle II by P. Goldenzweig                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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


