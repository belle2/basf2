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
#include <map>

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
     * decide between full or basic validation mode. full validation also produces global PID performance plots.
     */
    void validationMode(bool value = false) { m_fullValidation = value; }

    /**
     * function to enable plotting
     */
    void setMonitoringPlots(bool value = false) { m_isMakePlots = value; }

    /**
     * set the number of points for ROC curve plotting
     */
    void setNumROCpoints(const unsigned int& value) { m_NumROCpoints = value; }

    /**
     * set the lower edge of the momentum range for ROC curve plotting
     */
    void setMinROCMomentum(const double& value) { m_MomLowROC = value; }

    /**
     * set the lower edge of the momentum range for ROC curve plotting
     */
    void setMaxROCMomentum(const double& value) { m_MomHighROC = value; }

    /**
     * set the number of bins for the efficiency scan
     */
    void setNumEffBins(const unsigned int& value) { m_NumEffBins = value; }

    /**
     * set the upper edge of the momentum range for the efficiency scan
     */
    void setMaxEffMomentum(const double& value) { m_MomHighEff = value; }

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
    bool m_fullValidation;   /**< decide between full or basic validation mode. full validation also produces global PID performance plots.  */
    bool m_isMakePlots;                                                           /**< produce plots for monitoring of the fit quality */
    /**
     * a generic function to produce efficiency plots
     */
    void PlotEfficiencyPlots(const TString& PIDDetectorsName, TTree* SignalTree, TString SignalWeightName, TString SignalVarName,
                             TString SignalVarNameFull, TTree* FakeTree, TString FakeWeightName, TString FakeVarName, TString FakeVarNameFull,
                             TString PIDVarName, TString PIDCut, unsigned int nbins, double MomLow, double MomHigh);

    /**
     * a generic function to produce ROC curves
     */
    void PlotROCCurve(TTree* SignalTree, TString SignalWeightName, TString SignalVarName, TString SignalVarNameFull, TTree* FakeTree,
                      TString FakeWeightName, TString FakeVarName, TString FakeVarNameFull, TString PIDVarName);

    TTree* LambdaMassFit(std::shared_ptr<TTree> preselTree);  /**< produce histograms for protons */
    TTree* DstarMassFit(std::shared_ptr<TTree> preselTree); /**< produce histograms for K/pi(/mu) */

    int m_MinEvtsPerTree = 100;   /**< number of events in TTree below which we don't try to fit */
    unsigned int m_NumROCpoints = 250;    /**< number of points for the ROC curve plotting */
    double m_MomLowROC = 0.;    /**< lower edge of the momentum interval considered for the ROC curve */
    double m_MomHighROC = 7.;    /**< upper edge of the momentum interval considered for the ROC curve */
    unsigned int m_NumEffBins = 30;    /**< number of bins for the efficiency/fake rate plot */
    double m_MomHighEff = 2.5;    /**< upper edge of the momentum interval for the efficiency/fake rate plot */

  };
} // namespace Belle2
