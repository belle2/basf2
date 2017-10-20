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
    double qualityIndicator = 0;
    boost::optional<double> chiSquared;
    boost::optional<short> curvatureSign;
    boost::optional<double> pt;
    boost::optional<double> pmag;
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

    QualityEstimatorBase() {}

    virtual ~QualityEstimatorBase() = default;

    void setMagneticFieldStrength(double magneticFieldZ = 1.5) {m_magneticFieldZ = magneticFieldZ;}

    /** Minimal implementation of the quality estimation
     * Calculates quality indicator in range [0,1]
     *
     * measurements - std::vector<SpacePoint const*> ordered from innermost to outermost measurement
     */
    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) = 0;

    /** Quality estimation providing additional quantities
     * Calculates quality indicator in range [0,1]
     * Optionally returns chi2 and additional informations. Eg. momentum estimation.
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

    // Data members

    double m_magneticFieldZ = 1.5;

    /* This is stored as a member variable, because some values may be calculated by 'estimateQuality' anyways.
     * Therefore they don't need to be calculated explicitly in 'estimateQualityAndProperties'.
     */
    QualityEstimationResults m_results;
  };
}

