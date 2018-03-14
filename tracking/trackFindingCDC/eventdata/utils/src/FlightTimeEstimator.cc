/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/numerics/SpecialFunctions.h>
#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <framework/gearbox/Const.h>

#include <algorithm>

using namespace Belle2;
using namespace TrackFindingCDC;

const FlightTimeEstimator&
FlightTimeEstimator::instance(std::unique_ptr<FlightTimeEstimator> replacement)
{
  static std::unique_ptr<FlightTimeEstimator> s_instance(new FlightTimeEstimator);
  if (replacement) {
    s_instance = std::move(replacement);
  }
  return *s_instance;
}

namespace {
  double getFirstPeriodAlphaFlightTime(double absAlpha)
  {
    return 1.0 / (sinc(absAlpha) * Const::speedOfLight);
  }
}

BeamEventFlightTimeEstimator::BeamEventFlightTimeEstimator()
  : m_firstPeriodAlphaFlightTimeFactor(getFirstPeriodAlphaFlightTime,
                                       512,
                                       0,
                                       std::nextafter(M_PI, INFINITY))
{
}

namespace {
  double getHalfPeriodAlphaFlightTime(double absAlpha)
  {
    if (absAlpha > M_PI / 2.0) {
      double reverseAlpha = AngleUtil::reversed(absAlpha);
      return -1.0 / (sinc(reverseAlpha) * Const::speedOfLight);
    } else {
      return 1.0 / (sinc(absAlpha) * Const::speedOfLight);
    }
  }
}

CosmicRayFlightTimeEstimator::CosmicRayFlightTimeEstimator(Vector3D triggerPoint)
  : m_triggerPoint(triggerPoint)
  , m_halfPeriodAlphaFlightTimeFactor(getHalfPeriodAlphaFlightTime,
                                      512,
                                      0,
                                      std::nextafter(M_PI, INFINITY))
{
}
