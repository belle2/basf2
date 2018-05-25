/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun, Dmitrii Neverov                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/axes/DiscreteValue.h>
#include <tracking/trackFindingCDC/hough/axes/ContinuousValue.h>

#include <tracking/trackFindingCDC/topology/ILayer.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Phantom type tag for the discrete impact representation
    class ImpactTag;

    /// Type for discrete impact values
    using DiscreteImpact = DiscreteValue<float, ImpactTag>;

    /// Type to have explicit impact values
    using ContinuousImpact = ContinuousValue<double, ImpactTag>;

    /// Phantom type tag for the discrete curvature representation
    class CurvTag;

    /// Type for discrete curv values
    using DiscreteCurv = DiscreteValue<float, CurvTag>;

    /// Type to have explicit curv values
    using ContinuousCurv = ContinuousValue<double, CurvTag>;

    /**
     *  Class representing a curvature value that also caches two dimensional arc length
     *  to each layer in the CDC
     */
    class CurvWithArcLength2DCache {
    public:
      /// Make cache for one curvature value
      explicit CurvWithArcLength2DCache(float curv);

      /// Unpack the curvature
      explicit operator float() const
      {
        return m_curv;
      }

      /// Return the two dimensional arc length to the given layer id
      float getArcLength2D(ILayer iCLayer, bool secondArm = false) const
      {
        return secondArm ? m_secondaryArcLength2DByICLayer[iCLayer] : m_arcLength2DByICLayer[iCLayer];
      }

      /// Output operator for debugging
      friend std::ostream& operator<<(std::ostream& output, const CurvWithArcLength2DCache& value)
      {
        return output << value.m_curv;
      }

    private:
      /// Memory for the curvature
      float m_curv;

      /// Memory for two dimensional arc length at each layer.
      std::array<float, 56> m_arcLength2DByICLayer;

      /// Memory for two dimensional arc length at each layer on the second arm.
      std::array<float, 56> m_secondaryArcLength2DByICLayer;
    };

    /// Type for discrete curv values
    using DiscreteCurvWithArcLength2DCache = DiscreteValue<CurvWithArcLength2DCache, CurvTag>;

    /// Phantom type tag for the discrete tan lambda representation
    class TanLTag;

    /// Type for discrete tan lambda values
    using DiscreteTanL = DiscreteValue<float, TanLTag>;

    /// Type to have continuous tan lambda values
    using ContinuousTanL = ContinuousValue<double, TanLTag>;

    /// Phantom type tag for the discrete z0 representation
    class Z0Tag;

    /// Type for discrete phi0 values
    using DiscreteZ0 = DiscreteValue<float, Z0Tag>;

    /// Phantom type tag for the discrete phi0 representation
    class Phi0Tag;

    /// Type for discrete phi0 values
    using DiscretePhi0 = DiscreteValue<Vector2D, Phi0Tag>;

    /// Phantom type tag for the discrete p representation
    class PTag;

    /// Type for discrete p values
    using DiscreteP = DiscreteValue<float, PTag>;

    /// Type to have continuous p values
    using ContinuousP = ContinuousValue<double, PTag>;

    /// Phantom type tag for the discrete q representation
    class QTag;

    /// Type for discrete q values
    using DiscreteQ = DiscreteValue<float, QTag>;

    /// Type to have continuous q values
    using ContinuousQ = ContinuousValue<double, QTag>;

  }
}
