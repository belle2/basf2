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
#include <TVector3.h>


namespace Belle2 {

  /** Container for a 3D measurement in global coordinates including uncertainty. */
  struct Measurement {
    TVector3 const position;
    TVector3 const sigma;

    Measurement(TVector3 const& position, TVector3 const& sigma) : position(position), sigma(sigma) {}
  };

  /** Container for comlete fit results.
   * chiSquared is always computed, all other values are optional, depending on the implementation.
   */
  struct QualityEstimationResults {
    float chiSquared = 0;
    boost::optional<short> curvature;
    boost::optional<float> pt;
    boost::optional<float> pt_sigma;
    boost::optional<TVector3> p;
    boost::optional<TVector3> p_sigma;
  };

  /** BaseClass for QualityEstimators
   *
   * m_magneticFieldZ    - Z component of magnetic field
   * m_results           - Member object storing all optional results.
   * calcChiSquared      - Minimal implementation of the quality estimation
   * calcCompleteResults - Additionally calculates other properties that can be derived from the fit.
   */
  class QualityEstimatorBase {

  public:

    QualityEstimatorBase(double magneticFieldZ = 1.5):
      m_magneticFieldZ(magneticFieldZ) {}

    virtual ~QualityEstimatorBase() = default;

    virtual float calcChiSquared(std::vector<Measurement> const& measurements) = 0;

    virtual QualityEstimationResults calcCompleteResults(std::vector<Measurement> const& measurements)
    {
      m_results = QualityEstimationResults();
      m_results.curvature = calcCurvature(measurements);
      m_results.chiSquared = calcChiSquared(measurements);
      return m_results;
    }


  protected:

    /** Returns a value for the transverse momentun Pt in GeV calculated from a provided radius.
     *  Utilizing m_magneticFieldZ and hardcoded speed of light*/
    double calcPt(double radius) { return m_magneticFieldZ * radius * 0.00299792458; }

    /** Calculate curvature based on triplets of measurements.
     *  Ignores uncertainties.
     *  Returns -1,0,1 depending on the sum of all triplets.
     */
    short calcCurvature(std::vector<Measurement> const& measurements)
    {
      float sumOfCurvature = 0.;
      for (unsigned int i = 0; i < measurements.size() - 2; ++i) {
        TVector3 ab = measurements.at(i).position - measurements.at(i + 1).position;
        ab.SetZ(0.);
        TVector3 bc = measurements.at(i + 1).position - measurements.at(i + 2).position;
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

