/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  /** Enumeration of all TrackFinders, that might contribute CDC hits to a track. */
  enum class ETrackFinderIDCDC {
    c_global, c_local // , c_crossDetector, ...
  };

}
