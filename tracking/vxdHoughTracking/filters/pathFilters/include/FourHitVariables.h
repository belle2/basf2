/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/pathFilters/ThreeHitVariables.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <cmath>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Class that allows the calculation of simple variables to estimate the quality of two triplets of hits using four hits.
    class FourHitVariables {
    public:
      /// basic constructor
      FourHitVariables() : m_oHit(0., 0., 0.), m_ocHit(0., 0., 0.), m_icHit(0., 0., 0.), m_iHit(0., 0., 0.)
      {};

      /// actual useful constructor
      /// @param oHit B2Vector3D of the outer hit used for calculating the single variables
      /// @param ocHit B2Vector3D of the second-outer hit used for calculating the single variables
      /// @param icHit B2Vector3D of the second-inner hit used for calculating the single variables
      /// @param iHit B2Vector3D of the inner hit used for calculating the single variables
      FourHitVariables(const B2Vector3D& oHit, const B2Vector3D& ocHit, const B2Vector3D& icHit, const B2Vector3D& iHit) :
        m_oHit(oHit), m_ocHit(ocHit), m_icHit(icHit), m_iHit(iHit)
      {
        m_outerThreeHitVariables = ThreeHitVariables(oHit, ocHit, icHit);
        m_innerThreeHitVariables = ThreeHitVariables(ocHit, icHit, iHit);
      };

      /// Set hits if not given in constructor of if they need to be changed.
      /// @param oHit B2Vector3D of the outer hit used for calculating the single variables
      /// @param ocHit B2Vector3D of the second-outer hit used for calculating the single variables
      /// @param icHit B2Vector3D of the second-inner hit used for calculating the single variables
      /// @param iHit B2Vector3D of the inner hit used for calculating the single variables
      void setHits(const B2Vector3D& oHit, const B2Vector3D& ocHit, const B2Vector3D& icHit, const B2Vector3D& iHit)
      {
        m_oHit = oHit;
        m_ocHit = ocHit;
        m_icHit = icHit;
        m_iHit = iHit;
        m_outerThreeHitVariables = ThreeHitVariables(oHit, ocHit, icHit);
        m_innerThreeHitVariables = ThreeHitVariables(ocHit, icHit, iHit);
      }

      /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: cm */
      double getCircleRadiusDifference()
      {
        B2Vector3D outerCircleCenter = m_outerThreeHitVariables.getCircleCenterXY();
        B2Vector3D innerCircleCenter = m_innerThreeHitVariables.getCircleCenterXY();
        if (outerCircleCenter.Perp2() > 1e30 or innerCircleCenter.Perp2() > 1e30) {
          return NAN;
        }
        double outerCircleRadius = m_outerThreeHitVariables.calcAvgDistanceXY(outerCircleCenter);
        double innerCircleRadius = m_innerThreeHitVariables.calcAvgDistanceXY(innerCircleCenter);

        return outerCircleRadius - innerCircleRadius;
      }


      /** calculates the distance between the estimated circle centers (using 2 subsets of given hits) in the xy-plane, returning unit: cm */
      double getCircleCenterPositionDifference()
      {
        B2Vector3D outerCircleCenter = m_outerThreeHitVariables.getCircleCenterXY();
        B2Vector3D innerCircleCenter = m_innerThreeHitVariables.getCircleCenterXY();
        if (outerCircleCenter.Perp2() > 1e30 or innerCircleCenter.Perp2() > 1e30) {
          return NAN;
        }

        return fabs(outerCircleCenter.Perp() - innerCircleCenter.Perp());
      }


      /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
      double getDeltaPT()
      {
        B2Vector3D outerCircleCenter = m_outerThreeHitVariables.getCircleCenterXY();
        B2Vector3D innerCircleCenter = m_innerThreeHitVariables.getCircleCenterXY();
        if (outerCircleCenter.Perp2() > 1e30 or innerCircleCenter.Perp2() > 1e30) {
          return NAN;
        }
        double outerCircleRadius = m_outerThreeHitVariables.calcAvgDistanceXY(outerCircleCenter);
        double innerCircleRadius = m_innerThreeHitVariables.calcAvgDistanceXY(innerCircleCenter);

        return fabs(0.00299792458 * m_BFieldZ * (outerCircleRadius - innerCircleRadius));
      }

      /// Set the B-Field value used for pT calculations
      /// @param bfieldZ B-Field value to be used
      void setBFieldZ(const double bfieldZ = 1.5) { m_BFieldZ = bfieldZ; }

    private:
      /// BField along z to estimate pT
      double m_BFieldZ = 1.5;
      /// outermost hit position
      B2Vector3D m_oHit;
      /// second-to-outer hit position
      B2Vector3D m_ocHit;
      /// second-to-inner hit position
      B2Vector3D m_icHit;
      /// innermost hit position
      B2Vector3D m_iHit;
      /// ThreeHitVariables getter for the outer three hits
      ThreeHitVariables m_outerThreeHitVariables;
      /// ThreeHitVariables getter for the inner three hits
      ThreeHitVariables m_innerThreeHitVariables;

    };

  }
}
