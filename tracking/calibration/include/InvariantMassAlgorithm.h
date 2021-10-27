/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing InvariantMass calibration algorithm
   */
  class InvariantMassAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to BoostVectorCollector
    InvariantMassAlgorithm();

    /// Destructor
    virtual ~InvariantMassAlgorithm() {}

    /// Set outer loss function (for calibration intervals)
    void setOuterLoss(const std::string& loss) { m_lossFunctionOuter = loss; }

    /// Set inner loss function (for calibration subintervals)
    void setInnerLoss(const std::string& loss) { m_lossFunctionInner = loss; }

    /// Include calibration based on Hadronic B decays
    void includeHadBcalib(bool state = true) {m_runHadB = state;}

    /// Include calibration based on di-muons
    void includeMuMucalib(bool state = true) {m_runMuMu = state;}

    /// set the spread
    void setMuMuEcmsSpread(double spread) {m_eCMSmumuSpread = spread;}

    /// set the energy offset
    void setMuMuEcmsOffset(double shift) {m_eCMSmumuShift = shift;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /// Outer loss function (for calibration intervals with constant InvarinatMass spread)
    TString m_lossFunctionOuter = "pow(0.000020e1*rawTime, 2) +  1./nEv";

    /// Inner loss function (for calibration subintervals with constant InvariantMass)
    TString m_lossFunctionInner = "pow(0.000120e1*rawTime, 2) +  1./nEv";

    /// Run the calibration from had-B decays
    bool m_runHadB = true;

    /// Run the calibration from di-muons
    bool m_runMuMu = true;

    /// Energy spread for mumu only run (m_runHadB == false)
    double m_eCMSmumuSpread = 5.2e-3;

    /// Shift between the energy from the mumu events and the real value
    double m_eCMSmumuShift  = 10e-3;

  };
} // namespace Belle2

