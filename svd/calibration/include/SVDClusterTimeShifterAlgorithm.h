/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include "TMath.h"

#include <optional>

namespace Belle2 {
  /**
   * Class implementing SVDClusterTimeShifter algorithm
   */
  class SVDClusterTimeShifterAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor set the prefix to SVDTimeCalibrationCollector*/
    explicit SVDClusterTimeShifterAlgorithm(const std::string& str);

    /** Destructor*/
    virtual ~SVDClusterTimeShifterAlgorithm() {}

    /** Setter for m_allowedT0Shift*/
    void setAllowedTimeShift(const float& value) {m_allowedDeviationMean = value;}

    /** set list of time algorithm */
    void setTimeAlgorithm(const std::vector<TString>& lst) {m_timeAlgorithms = lst;}

    /** Set the minimum entries required in the histograms */
    void setMinEntries(const int& minEntries) {m_minEntries = minEntries;}

    /** Set algorithm for creating payload boundary */
    void setTimeAlgorithmForIoV(const TString& alg) {m_timeAlgorithmForIoV = alg;}

  protected:

    /** Run algo on data*/
    virtual EResult calibrate() override;

  private:

    /** If the event T0 changes significantly return true. This is run inside the findPayloadBoundaries member function
    in the base class. */
    virtual bool isBoundaryRequired(const Calibration::ExpRun& currentRun) override;

    /** setup the boundary finding*/
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) override
    {
      m_previousTimeMeanL3V.reset();
    }

    std::string m_id = ""; /**< Parameter given to set the UniqueID of the payload*/
    std::optional<float> m_previousTimeMeanL3V; /**< CoG of the previous run*/
    float m_allowedDeviationMean = 15; /**< Allowed deviation of clsOnTracks histo wrt EventT0 histo */
    int m_minEntries = 1000; /**< Set the minimun number of entries required in the histograms*/
    std::vector<TString> m_timeAlgorithms = {"CoG3", "ELS3", "CoG6"}; /**< List of time algorithms to calibrate */
    TString m_timeAlgorithmForIoV = "CoG3"; /**< Histogram of one algorithm is used to produce payload boundaries */
  };

  /** single gaus function.
   * @param N is a normalization constant
   * @param a is the mean of the Gaus distribution
   * @param b is the standard deviation of the Gaus distribution
   * @param e is a contant
   * @return value of single gaus distribution
   */
  inline double singleGaus(const double* x, const double* par)
  {
    double N = std::fabs(par[0]);
    double a = par[1];
    double b = par[2];
    double e = std::fabs(par[3]);
    return N * TMath::Gaus(x[0], a, b, true) + e;
  };

  /** double gaus function.
   * @param N is a normalization constant
   * @param f is the fractional contribution of the first gaus distribution
   * @param a is the mean of the first gaus distribution
   * @param b is the standard deviation of the first gaus distribution
   * @param c is the mean of the second gaus distribution
   * @param d is the standard deviation of the second gaus distribution
   * @param e is a contant
   * @return value of double gaus distribution
   */
  inline double doubleGaus(const double* x, const double* par)
  {
    double N = std::fabs(par[0]);
    double f = std::fabs(par[1]);
    double a = par[2];
    double b = par[3];
    double c = par[4];
    double d = par[5];
    double e = std::fabs(par[6]);
    return (N * (f * TMath::Gaus(x[0], a, b) + (1 - f) * TMath::Gaus(x[0], c, d)) /
            (TMath::Sqrt(2. * TMath::Pi()) * ((b - d) * f + d)) + e);
  };

} // namespace Belle2
