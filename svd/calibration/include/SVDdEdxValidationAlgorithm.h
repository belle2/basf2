/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <TTree.h>

namespace Belle2 {
  /**
   * Class implementing the SVD dEdx calibration algorithm
   */
  class SVDdEdxValidationAlgorithm : public CalibrationAlgorithm {
  public:
    /**
     * Constructor
     */

    SVDdEdxValidationAlgorithm();

    /**
     * Destructor
     */

    virtual ~SVDdEdxValidationAlgorithm() {}

    /**
     * function to enable plotting
     */
    void setMonitoringPlots(bool value = false) { m_isMakePlots = value; }

    // /**
    //  * set the number of dEdx bins for the payloads
    //  */
    // void setNumDEdxBins(const int& value) { m_numDEdxBins = value; }

    // /**
    //  * set the number of momentum bins for the payloads
    //  */
    // void setNumPBins(const int& value) { m_numPBins = value; }

    // /**
    //  * set the upper edge of the dEdx binning for the payloads
    //  */
    // void setDEdxCutoff(const double& value) { m_dedxCutoff = value; }

    /**
     * set the upper edge of the dEdx binning for the payloads
     */
    void setMinEvtsPerTree(const double& value) { m_MinEvtsPerTree = value; }

  protected:
    /**
     * run algorithm on data
     */
    virtual EResult calibrate() override;

  private:
    bool m_isMakePlots;                                                           /**< produce plots for monitoring of the fit quality */
    /**
     * a generic function to produce efficiency plots
     */
    void PlotEfficiencyPlots(const TString& LayerName, TTree* SignalTree, TString SignalWeightName, TString SignalVarName,
                             TString SignalVarNameFull, TTree* FakeTree, TString FakeWeightName, TString FakeVarName, TString FakeVarNameFull,
                             TString PIDVarName, TString PIDCut, unsigned int nbins, double MomLow, double MomHigh);

    /**
     * a generic function to produce ROC curves
     */
    void PlotROCCurve(TTree* SignalTree, TString SignalWeightName, TString SignalVarName, TString SignalVarNameFull, TTree* FakeTree,
                      TString FakeWeightName, TString FakeVarName, TString FakeVarNameFull, TString PIDVarName, double MomLow, double MomHigh);

    TTree* LambdaMassFit(std::shared_ptr<TTree> preselTree);  /**< produce histograms for protons */
    TTree* DstarMassFit(std::shared_ptr<TTree> preselTree); /**< produce histograms for K/pi(/mu) */

    int m_MinEvtsPerTree = 100;   /**< number of events in TTree below which we don't try to fit */

  };
} // namespace Belle2
