/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include <TMath.h>

#include <optional>

namespace Belle2 {
  /**
   * Class implementing SVDClusterTimeShifter algorithm
   */
  class SVDClusterAbsoluteTimeShifterAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor set the prefix to SVDTimeCalibrationCollector*/
    explicit SVDClusterAbsoluteTimeShifterAlgorithm(const std::string& str);

    /** Destructor*/
    virtual ~SVDClusterAbsoluteTimeShifterAlgorithm() {}

    /** Setter for m_allowedTimeShift */
    void setAllowedTimeShift(float value) {m_allowedTimeShift = value;}

    /** Getter for m_allowedTimeShift */
    float getAllowedTimeShift() {return m_allowedTimeShift;}

    /** Setter for m_allowedT0Shift*/
    void setMaximumAllowedShift(const float& value) {m_maximumAllowedShift = value;}

    /** Get the maximum allowed deviation of clsOnTracks histo wrt EventT0 histo (m_allowedT0Shift)*/
    float getMaximumAllowedShift() {return m_maximumAllowedShift;}

    /** Set list of time algorithm */
    void setTimeAlgorithm(const std::vector<TString>& lst) {m_timeAlgorithms = lst;}

    /** Get list of time algorithm */
    std::vector<TString> getTimeAlgorithm() {return m_timeAlgorithms;}

    /** Set the minimum entries required in the histograms */
    void setMinEntries(const int& minEntries) {m_minEntries = minEntries;}

    /** Get the minimum entries required in the histograms */
    int getMinEntries() {return m_minEntries;}

  protected:

    /** Run algo on data*/
    virtual EResult calibrate() override;

    /** If the event T0 changes significantly return true. This is run inside the findPayloadBoundaries member function
    in the base class. */
    virtual bool isBoundaryRequired(const Calibration::ExpRun& currentRun) override;

    /** setup the boundary finding*/
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) override
    {
      m_previousTimeMeanL3V.reset();
    }


  private:

    std::string m_id = ""; /**< Parameter given to set the UniqueID of the payload*/
    std::optional<float> m_previousTimeMeanL3V; /**< Cluster time of the previous run*/
    float m_allowedTimeShift = 2.; /**< Allowed Raw CoGshift*/
    float m_maximumAllowedShift = 15.; /**< Allowed deviation of clsOnTracks histo wrt EventT0 histo */
    int m_minEntries = 100; /**< Set the minimum number of entries required in the histograms*/
    std::vector<TString> m_timeAlgorithms = {"CoG3", "ELS3", "CoG6"}; /**< Hardcoded list of time algorithms to calibrate */
    int m_innerLayer = 3; /**< Inner layer of the SVD */
    int m_outerLayer = 6; /**< Outer layer of the SVD */
  };

  /** Single gauss function, where
   * N is a normalization constant,
   * a is the mean of the Gauss distribution,
   * b is the standard deviation of the Gauss distribution,
   * e is a constant.
   * @param x independent variable of the function
   * @param par parameters of the function
   * @return value of single gauss distribution
   * Need to slightly rename it because it clashes with same function definition in SVDClusterTimeShifterAlgorithm
   *
   */
  inline double AbssingleGaus(const double* x, const double* par)
  {
    double N = std::fabs(par[0]);
    double a = par[1];
    double b = par[2];
    double e = std::fabs(par[3]);
    return N * TMath::Gaus(x[0], a, b, true) + e;
  };

  /** Double gauss function, where
   * N is a normalization constant,
   * f is the fractional contribution of the first gauss distribution,
   * a is the mean of the first gauss distribution,
   * b is the standard deviation of the first gauss distribution,
   * c is the mean of the second gauss distribution,
   * d is the standard deviation of the second gauss distribution,
   * e is a constant.
   * @param x independent variable of the function
   * @param par parameters of the function
   * @return value of double gauss distribution
   */
  inline double AbsdoubleGaus(const double* x, const double* par)
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
