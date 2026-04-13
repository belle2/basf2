/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/dbobjects/CDCDedxCosineCor.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <TH1D.h>
#include <array>
#include <vector>
#include <TLegend.h>

namespace Belle2 {

  /**
  * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
  */
  class CDCDedxCosLayerAlgorithm : public CalibrationAlgorithm {

  public:

    static constexpr int m_kNGroups = 3; /**< SL grouping: inner (SL0), middle (SL1), outer (SL2–8) */

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxCosLayerAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxCosLayerAlgorithm() {}

    /**
    * function to set truncation method (local vs global)
    */
    void setLayerTrunc(bool value = false) {isFixTrunc = value;}

    /**
    * function to decide merge vs relative gains
    */
    void setMergePayload(bool value = true) {isMerge = value;}

    /**
    * function to make flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

    /**
    * function to make flag active for method of sep
    */
    void setMethodSep(bool value = true) {isMethodSep = value;}

    /**
    * Enable/disable truncated mean usage for inner superlayers (e.g. SL0–1)
    */
    void setUseTruncForInner(bool value = false) { isUseTrunc = value; }

    /**
    * function to set bins of truncation from histogram
    */
    void setTrucationBins(double lowedge, double upedge)
    {
      m_truncMin = lowedge; m_truncMax = upedge ;
    }

    /**
    * function to set number of cosine bins for calibration
    */
    void setCosineBins(unsigned int value = 100) {m_cosBin = value;}

    /**
    * function to set min/max range of cosine for calibration
    */
    void setCosineRange(double min = -1.0, double max = 1.0) {m_cosMin = min; m_cosMax = max;}

    /**
    * function to set nbins of dedx dist calibration
    */
    void setHistBins(int value = 250) {m_dedxBin = value;}

    /**
    * function to set min/max range of dedx dist calibration
    */
    void setHistRange(double min = 0.0, double max = 5.0) {m_dedxMin = min; m_dedxMax = max;}

    /**
    * adding suffix to control plots
    */
    void setSuffix(const std::string& value) {m_suffix = value;}

    /**
    * Representative CDC layer for each SL group (used to access group-wise constants):
    * SL0 => 1, SL1 => 9, SL2-8 => 17
    */
    unsigned int getRepresentativeLayer(unsigned int igroup) const
    {
      static const std::array<unsigned int, m_kNGroups> repLayer = {1, 9, 17};
      return repLayer.at(igroup);
    }

    /**
    * function to get extract calibration run/exp
    */
    void getExpRunInfo();

    /**
    * function to define dE/dx histograms
    */
    void defineHisto(std::array<std::vector<TH1D*>, m_kNGroups>& hdedx, const std::string& tag, const std::string& chargeLabel);

    /**
    * function to define cosine histograms
    */
    TH1D* defineCosthHist(const std::string& tag, const std::string& chargeLabel);

    /**
    * function to get bins of truncation from histogram
    */
    void getTruncatedBins(TH1D* hist, int& binlow, int& binhigh);

    /**
    * function to get truncated mean
    */
    double getTruncationMean(TH1D* hist, int binlow, int binhigh);

    /**
    * Extract mean dE/dx vs costh for a given group from the histogram
    */
    double extractCosMean(TH1D*& hist, int fixedLow = 1, int fixedHigh = 1);

    /**
    * function to store new payload after full calibration
    */
    void createPayload();

    /**
    * function to draw the dE/dx histogram in costh bins
    */
    void plotdedxHist(std::array<std::vector<TH1D*>, m_kNGroups>& hdedx, const std::string& tag);

    /**
    * function to draw dedx dist. for Inner/outer layer
    */
    void plotLayerDist(std::array<TH1D*, m_kNGroups>& hdedxlay, int iter);

    /**
    * function to costh distribution for Inner/Outer layer
    */
    void plotQaPars(TH1D* hCosth_all, TH1D* hCosth_pos, TH1D* hCosth_neg);

    /**
    * Plot relative calibration constants vs costh for all SL groups (overlayed)
    */
    void plotRelConst(const std::array<std::vector<double>, m_kNGroups>& cosine, int iter);

    /**
    * Plot overlay of positive, negative, and average cosine means for one SL group
    */
    void plotmeanChargeOverlay(const std::array<std::vector<double>, 3>& cosine_pos, const std::string& sltag, int iter);

    /**
    * function to draw the old/new final constants
    */
    void plotConstants();

    /**
     * function to draw the stats plots
     */
    void plotEventStats();


  protected:

    /**
    * Cosine algorithm
    */
    virtual EResult calibrate() override;

  private:
    bool isMethodSep; /**< if e+e- need to be consider sep */
    bool isMakePlots; /**< produce plots for status */
    bool isMerge; /**< merge payload at the of calibration */
    bool isFixTrunc; /**< true = fix window for all out/inner layers */
    bool isUseTrunc; /**< true if truncated mean for SL0,1 */

    double m_truncMin; /**< lower threshold on truncation*/
    double m_truncMax; /**< upper threshold on truncation */

    unsigned int m_cosBin; /**< number of bins across cosine range */
    double m_cosMin; /**< min cosine angle for cal */
    double m_cosMax; /**< max cosine angle for cal */

    int m_dedxBin; /**< number of bins for dedx histogram */
    double m_dedxMin; /**< min dedx range for gain cal */
    double m_dedxMax; /**< max dedx range for gain cal */

    std::string m_suffix; /**< add suffix to all plot name  */
    std::string m_runExp; /**< add run and exp to title of plot  */

    const std::array<std::string, m_kNGroups> m_label = {"SL0", "SL1", "SL2-8"}; /**< add inner/outer superlayer label */

    std::vector<std::vector<double>> m_coscors; /**< final vectors of calibration  */

    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */

  };
} // namespace Belle2
