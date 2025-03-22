/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/numerics/EForwardBackward.h>
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
  constexpr inline bool arcLengthInRightDirection(double arcLength2D, TrackingUtilities::EForwardBackward forwardBackward)
  {
    return static_cast<double>(forwardBackward) * arcLength2D >= 0;
  }

  /// Helper function to turn a direction string into a valid forward backward information.
  inline TrackingUtilities::EForwardBackward fromString(const std::string& directionString)
  {
    if (directionString == "forward" or directionString == "above" or directionString == "after") {
      return TrackingUtilities::EForwardBackward::c_Forward;
    } else if (directionString == "backward" or directionString == "below" or directionString == "before") {
      return TrackingUtilities::EForwardBackward::c_Backward;
    } else if (directionString == "both" or directionString == "unknown") {
      return TrackingUtilities::EForwardBackward::c_Unknown;
    } else if (directionString == "none" or directionString == "invalid") {
      return TrackingUtilities::EForwardBackward::c_Invalid;
    } else {
      B2FATAL("Do not understand direction " << directionString << ". Valid names are " <<
              "forward/above/after, backward/below/before, both/unknown, none/invalid");
    }
  }
}
