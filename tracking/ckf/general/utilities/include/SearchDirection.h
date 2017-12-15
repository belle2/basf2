/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <framework/logging/Logger.h>

#include <string>

namespace Belle2 {
  /**
   * Given the calculated arc length between a start point and an end point, checks if the travelled
   * path is along the given direction or not.
   *
   * If the arc length distance is negative, the end point has a higher arc length value than the start point meaning
   * it is farther away -> the travelled distance is forward.
   * If the arc length distance is positive, it is backward.
   *
   * If it is 0 or the direction is unknown, true is always returned.
   */
  constexpr inline bool arcLengthInRightDirection(double arcLength2D, TrackFindingCDC::EForwardBackward forwardBackward)
  {
    return forwardBackward * arcLength2D >= 0;
  }

  /// Helper function to turn a direction string into a valid forward backward information.
  inline TrackFindingCDC::EForwardBackward fromString(const std::string& directionString)
  {
    if (directionString == "forward" or directionString == "above" or directionString == "after") {
      return TrackFindingCDC::EForwardBackward::c_Forward;
    } else if (directionString == "backward" or directionString == "below" or directionString == "before") {
      return TrackFindingCDC::EForwardBackward::c_Backward;
    } else if (directionString == "both" or directionString == "unknown") {
      return TrackFindingCDC::EForwardBackward::c_Unknown;
    } else {
      B2FATAL("Do not understand direction " << directionString << ". Valid names are " <<
              "forward/above/after, backward/below/before, both/unknown");
    }
  }
}