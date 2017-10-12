/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/numerics/LookupTable.h>

#include <memory>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Helper struct to provide consistent flight time estimation throughout the CDC track finding.
     */
    class FlightTimeEstimator {

    public:
      /// Making the destructor virtual for this interface class
      virtual ~FlightTimeEstimator() = default;

      /// Default estimator for the flight time
      virtual double
      getFlightTime2D(const Vector2D& /*pos2D*/, double /*alpha*/, double /*beta */ = 1) const
      {
        return 0;
      }

    public:
      /// Getter for the instance
      static const FlightTimeEstimator&
      instance(std::unique_ptr<FlightTimeEstimator> replacement = nullptr);
    };

    /// Concrete estimator for the regular beam event setup
    class BeamEventFlightTimeEstimator : public FlightTimeEstimator {

    public:
      /// Constructor also setting up the flight time lookup table
      BeamEventFlightTimeEstimator();

      /// Flight time estimator for regular beam events
      double getFlightTime2D(const Vector2D& pos2D, double alpha, double beta = 1) const override
      {
        double absAlpha = std::fabs(alpha);
        double directDist2D = pos2D.cylindricalR();
        return directDist2D * m_firstPeriodAlphaFlightTimeFactor(absAlpha) / beta;
      }

    private:
      /// Lookup table for the sinc function
      LookupTable<float> m_firstPeriodAlphaFlightTimeFactor;
    };

    /// Concrete estimator for the cosmic ray setup - estimates negative times on incoming arm
    class CosmicRayFlightTimeEstimator : public FlightTimeEstimator {

    public:
      /// Constructor also setting up the flight time lookup table
      explicit CosmicRayFlightTimeEstimator(Vector3D triggerPoint = Vector3D(0, 0, 0));

      /// Flight time estimator for cosmic ray events
      double getFlightTime2D(const Vector2D& pos2D, double alpha, double beta = 1) const override
      {
        Vector2D relPos2D = pos2D - m_triggerPoint.xy();
        double deltaAlpha = pos2D.angleWith(relPos2D);
        alpha += deltaAlpha;
        double absAlpha = std::fabs(alpha);
        double directDist2D = relPos2D.cylindricalR();
        return directDist2D * m_halfPeriodAlphaFlightTimeFactor(absAlpha) / beta;
      }

    private:
      /// Trigger point of the cosmic ray setup
      Vector3D m_triggerPoint;

      /// Lookup table for the sinc function
      LookupTable<float> m_halfPeriodAlphaFlightTimeFactor;
    };

  }
}
