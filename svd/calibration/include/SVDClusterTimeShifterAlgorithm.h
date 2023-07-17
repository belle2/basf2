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

  protected:

    /** Run algo on data*/
    virtual EResult calibrate() override;


  private:

    std::string m_id = ""; /**< Parameter given to set the UniqueID of the payload*/
    float m_allowedDeviationMean = 15; /**< Allowed deviation of clsOnTracks histo wrt EventT0 histo */
    int m_minEntries = 1000; /**< Set the minimun number of entries required in the histograms*/
    std::vector<TString> m_timeAlgorithms = {"CoG3", "ELS3", "CoG6"}; /**< List of time algorithms to calibrate */
  };

  /** return a double gaus*/
  inline double myDoubleGaus(const double* x, const double* par)
  {
    double N = par[0];
    double f = par[1];
    double a = par[2];
    double b = par[3];
    double c = par[4];
    double d = par[5];
    double e = par[6];
    return (N * (f * TMath::Gaus(x[0], a, b) + (1 - f) * TMath::Gaus(x[0], c, d)) /
            (TMath::Sqrt(2. * TMath::Pi()) * ((b - d) * f + d)) + e);
  };

} // namespace Belle2
