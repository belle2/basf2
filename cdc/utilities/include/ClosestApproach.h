/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/geometry/B2Vector3.h>

namespace Belle2 {
  namespace CDC {
    /**
     * Returns a closest distance between a track and a wire.
     * @param[in]  bwp wire position at backward
     * @param[in]  fwp wire position at forward
     * @param[in]  posIn entrance position
     * @param[in]  posOut exit position
     * @param[out] hitPosition track position corresp. to the closetst distance
     * @param[out] wirePosition wire position corresp. to the closetst distance
     */
    double ClosestApproach(const B2Vector3D& bwp, const B2Vector3D& fwp, const B2Vector3D& posIn, const B2Vector3D& posOut,
                           B2Vector3D& hitPosition, B2Vector3D& wirePosition);

  } // end of namespace CDC
} // end of namespace Belle2


