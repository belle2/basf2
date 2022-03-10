/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <tracking/calibration/InvariantMassMuMuStandAlone.h>
#include <tracking/calibration/InvariantMassBhadStandAlone.h>
#include <tracking/calibration/calibTools.h>

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

    /// set the spread
    void setMuMuEcmsSpread(double spread) {m_eCMSmumuSpread = spread;}

    /// set the energy offset
    void setMuMuEcmsOffset(double shift) {m_eCMSmumuShift = shift;}

    /// Load the mumu data from files
    std::vector<InvariantMassMuMuCalib::Event> getDataMuMu(const std::vector<std::string>& files, bool is4S);

    /// Load the hadB data from files
    std::vector<InvariantMassBhadCalib::Event> getDataHadB(const std::vector<std::string>& files);

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    /// Adjust the energy of the off-resonance runs based on the energy offset between mumu and hadB method in the neighboring blocks
    std::vector<std::vector<CalibrationData>> adjustOffResonanceEnergy(std::vector<std::vector<CalibrationData>>  CalResultsBlocks,
                                           const std::vector<std::vector<InvariantMassMuMuCalib::Event>>&  evtsMuMuBlocks);


    /// Outer loss function (for calibration intervals with constant InvarinatMass spread)
    TString m_lossFunctionOuter = "pow(0.000010e0*rawTime, 2) +  1./nEv";

    /// Inner loss function (for calibration subintervals with constant InvariantMass)
    TString m_lossFunctionInner = "pow(0.000120e0*rawTime, 2) +  1./nEv";

    /// Run the calibration from had-B decays
    bool m_runHadB = true;

    /// Energy spread for mumu only run (m_runHadB == false)
    double m_eCMSmumuSpread = 5.2e-3;

    /// Shift between the energy from the mumu events and the real value
    double m_eCMSmumuShift  = 10e-3;

  };
} // namespace Belle2

