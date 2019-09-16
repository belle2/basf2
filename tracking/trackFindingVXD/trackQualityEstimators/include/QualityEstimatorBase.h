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

#include <framework/geometry/B2Vector3.h>
#include <tracking/spacePointCreation/SpacePoint.h>


namespace Belle2 {

  /** Container for complete fit/estimation results.
   * qualityIndicator is always computed, all other values are optional, depending on the implementation.
   */
  struct QualityEstimationResults {
    double qualityIndicator = 0; /**< return value of the quality estimator */
    boost::optional<double> chiSquared; /**< chi squared value obtained by the fit of the QE */
    boost::optional<short> curvatureSign; /**< direction of curvature as obtained by the QE */
    boost::optional<double> pt; /**< transverse momentum estimate from the QE */
    boost::optional<double> pmag; /**< momentum magnitute estimate from the QE */
    boost::optional<B2Vector3<double>> p; /**< momentum vector estimate from the QE */
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
    /// Constructor
    QualityEstimatorBase() {}

    /// Destructor
    virtual ~QualityEstimatorBase() = default;

    /** Setter for z component of magnetic field
     * @param magneticFieldZ : value to set it to
     */
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
    /// Member storing the z component of the magnetic field
    double m_magneticFieldZ = 1.5;

    /** Result of the quality estimation
     * This is stored as a member variable, because some values may be calculated by 'estimateQuality' anyways.
     * Therefore they don't need to be calculated explicitly in 'estimateQualityAndProperties'.
     */
    QualityEstimationResults m_results;
  };
}

