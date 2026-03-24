/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace ECL {

    //** Enumurate ECL detector regions */
    enum DetectorRegion {outsideAcceptance = 0, FWD = 1, BRL = 2, BWD = 3, FWDGap = 11, BWDGap = 13};

    /** Return ECL detector region according to theta */
    DetectorRegion getDetectorRegion(const double theta);

  } // namespace ECL
} // namespace Belle2


