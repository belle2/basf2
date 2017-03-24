/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/optional.hpp>
#include <math.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>



namespace Belle2 {

  /** Container for complete fit/estimation results.
   * qualityIndicator is always computed, all other values are optional, depending on the implementation.
   */
  struct QualityEstimationResults {
    double qualityIndicator;
    boost::optional<double> chiSquared;
    boost::optional<short> curvatureSign;
    boost::optional<double> pt;
    boost::optional<B2Vector3<double>> p;
  };

  /** BaseClass for QualityEstimators
   *
   * m_magneticFieldZ             - Z component of magnetic field
   * m_results                    - Member object storing all optional results.
   * estimateQuality              - Minimal implementation of the quality estimation
   * estimateQualityAndProperties - Additionally calculates other properties that can be derived from the fit.
   */
  class QualityEstimatorBase {

  public:

    QualityEstimatorBase(double magneticFieldZ = 1.5):
      m_magneticFieldZ(magneticFieldZ) {}

    virtual ~QualityEstimatorBase() = default;

    /** Minimal implementation of the quality estimation
     * Calculates quality indicator in range [0,1]
     *
     * measurements - std::vector<SpacePoint const*> ordered from innermost to outermost measurement
     */
    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) = 0;

    /** Quality estimation providing additional quantities
     * Calculates quality indicator in range [0,1]
     * Optionally returns chi2, curvatureSign , pt, p
     *
     * measurements - std::vector<SpacePoint const*> ordered from innermost to outermost measurement
     */
    virtual QualityEstimationResults estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements)
    {
      m_results = QualityEstimationResults();
      m_results.qualityIndicator = estimateQuality(measurements);
      return m_results;
    }


  protected:

    // utility methods

    /** Returns a value for the transverse momentum in GeV calculated from a provided radius.
     *  Utilizing m_magneticFieldZ and hard coded speed of light*/
    double calcPt(double const radius) const { return m_magneticFieldZ * radius * 0.00299792458; }

    /** Calculate curvature based on triplets of measurements.
     *  Ignores uncertainties.
     *  Returns -1,0,1 depending on the sum of all triplets.
     */
    short calcCurvatureSign(std::vector<SpacePoint const*> const& measurements) const
    {
      if (measurements.size() < 3) return 0;
      float sumOfCurvature = 0.;
      for (unsigned int i = 0; i < measurements.size() - 2; ++i) {
        TVector3 ab = measurements.at(i)->getPosition() - measurements.at(i + 1)->getPosition();
        ab.SetZ(0.);
        TVector3 bc = measurements.at(i + 1)->getPosition() - measurements.at(i + 2)->getPosition();
        bc.SetZ(0.);
        sumOfCurvature += bc.Orthogonal() * ab; //normal vector of m_vecBC times segment of ba
      }
      if (sumOfCurvature > 0) return 1;
      if (sumOfCurvature < 0) return -1;
      else return 0;
    }

    // Data members

    double m_magneticFieldZ;

    QualityEstimationResults m_results;
  };
}

