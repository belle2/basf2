/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Calculation of shower positions using lists of digits and weights.     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alon Hershenhorn (hershen@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace ECL {

    //** Enumurate ECL detector regions */
    enum DetectorRegion {outsideAcceptance = 0, FWD = 1, BRL = 2, BWD = 3, FWDGap = 11, BWDGap = 13};

    /** Return ECL detector region according to theta */
    DetectorRegion getDetectorRegion(const double theta)
    {
      if (theta < 0.2164208) return DetectorRegion::outsideAcceptance;   // < 12.4deg
      if (theta < 0.5480334) return DetectorRegion::FWD;   // < 31.4deg
      if (theta < 0.561996) return DetectorRegion::FWDGap;   // < 32.2deg
      if (theta < 2.2462387) return DetectorRegion::BRL;   // < 128.7deg
      if (theta < 2.2811453) return DetectorRegion::BWDGap;   // < 130.7deg
      if (theta < 2.7070057) return DetectorRegion::BWD;   // < 155.1deg
      return DetectorRegion::outsideAcceptance;
    }

  } // namespace ECL
} // namespace Belle2


