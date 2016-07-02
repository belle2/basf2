/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "TVector3.h"

namespace Belle2 {
  namespace CDC {
    /**
     * Returns a closest distance between a track and a wire.
     * @param bwp[in] wire position at backward
     * @param fwp[in] wire position at forward
     * @param posIn[in] entrance position
     * @param posOut[in] exit position
     * @param hitPosition[out] track position corresp. to the closetst distance
     * @param wirePosition[out] wire position corresp. to the closetst distance
     */
    double ClosestApproach(const TVector3& bwp, const TVector3& fwp, const TVector3& posIn, const TVector3& posOut,
                           TVector3& hitPosition, TVector3& wirePosition);

  } // end of namespace CDC
} // end of namespace Belle2


