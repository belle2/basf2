/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TH1D.h>
#include <TH2D.h>

#include <cdc/dbobjects/CDCDedx1DCell.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  /**
   * A calibration algorithm for CDC dE/dx electron: 1D enta cleanup correction
   *
   */
  class CDCDedx1DCellAlgorithm : public CalibrationAlgorithm {

  public:

    static constexpr int m_kNGroups = 3; /**< SL grouping: inner (SL0), middle (SL1), outer (SL2–8) */

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedx1DCellAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedx1DCellAlgorithm() {}

    /**
    * function to set min/max range of entrance angle for calibration
    */
    void setEntaRange(double min = -1.0, double max = 1.0) {m_eaMin = min; m_eaMax = max;}

    /**
    * function to set number of entrance angle bins for calibration
    */
    void setEntaBins(unsigned int value = 316) {m_eaB = value;}

    /**
    * function to set nbins of dedx dist for calibration
    */
    void setHistBins(int value = 250) {m_dedxBin = value;}

    /**
    * function to set min/max range of dedx dist for calibration
    */
    void setHistRange(double min = 0.0, double max = 5.0) {m_dedxMin = min; m_dedxMax = max;}

    /**
    * function to set pt limit
    */
    void setPtLimit(double value) {m_ptMax = value;}

    /**
    * function to set costheta limit
    */
    void setCosLimit(double value) {m_cosMax = value;}

    /**
    * function to set bins of truncation from histogram
    */
    void setTrucationBins(double lowedge, double upedge)
    {
      m_truncMin = lowedge; m_truncMax = upedge ;
    }

    /**
    * set bin split factor for all range
    */
    void setSplitFactor(int value) {m_binSplit = value;}

    /**
    * set charge type
    */
    void setChargeType(int value) {m_chargeType = value;}

    /**
    * set adjustment factor
    */
    void setAdjustmentFactor(int value) {m_adjustFac = value;}

    /**
    * function to set truncation method (local vs global)
    */
    void setLayerTrunc(bool value = false) {isFixTrunc = value;}

    /**
    * Set Var bins flag to on or off
    */
    void setVariableBins(bool value) {isVarBins = value;}

    /**
    * set rotation sys to copy constants from one region to other
    */
    void setRotSymmetry(bool value) {isRotSymm = value;}

    /**
    * funtion to set flag active for plotting
    */
    void enableExtraPlots(bool value = false) {isMakePlots = value;}

    /**
    * funtion to set flag to print log
    */
    void setPrintLog(bool value = false) {isPrintLog = value;}

    /**
    * set false if generating absolute (not relative)
    * payload
    */
    void setMergePayload(bool value) { isMerge = value;}

    /**
    * adding suffix to control plots
    */
    void setSuffix(const std::string& value) {m_suffix = value;}

    /**
    * Representative CDC layer for each SL group (used to access group-wise constants):
    * SL0 => 1, SL1 => 9, SL2-8 => 17
    */
    unsigned int getRepresentativeLayer(unsigned int il) const
    {
      static const std::array<unsigned int, m_kNGroups> repLayer = {1, 9, 17};
      return repLayer.at(il);
    }

    /**
    * adjust baseline based on charge or global overall
    * works for only single charge or both
    */
    void setBaselineFactor(double charge, double factor)
    {

      m_adjustFac = factor;
      if (charge < 0)m_chargeType = -1.0;
      else if (charge > 0)m_chargeType = 1.0;
      else if (charge == 0)m_chargeType = 0.0;
      else
        B2FATAL("Choose charge value either +/-1 or 0");
    }

    /**
    * class function to set rotation symmetry
    */
    int rotationalBin(int nbin, int ibin)
    {
      if (nbin % 4 != 0)return ibin;
      int jbin = ibin;
      if (ibin < nbin / 4) jbin = ibin + nbin / 2 ;
      else if (ibin >= 3 * nbin / 4) jbin = ibin - nbin / 2 ;
      return jbin;
    }

    /**
    * function to extract calibration run/exp
    */
    void getExpRunInfo();

    /**
    * class function to create vectors for bin mapping (Var->symm)
    */
    void CreateBinMapping();

    /**
    * function to define histograms
    */
    void defineHisto(std::array<std::vector<TH1D*>, 3>& hdedxhit, std::array<TH1D*, 3>& hdedxlay, std::array<TH1D*, 3>& hentalay);

    /**
    * function to get bins of truncation from histogram
    */
    void getTruncatedBins(TH1D* hist, int& binlow, int& binhigh);

    /**
    * function to get truncated mean
    */
    double getTruncationMean(TH1D* hist, int binlow, int binhigh);

    /**
    * function to generate final constants
    */
    void createPayload();

    /**
    * function to plot merging factor
    */
    void plotMergeFactor(std::map<int, std::vector<double>> bounds, const std::array<int, 2> nDev,
                         std::map<int, std::vector<int>> steps);

    /**
    * function to draw the dE/dx histogram in enta bins
    */
    void plotdedxHist(std::array<std::vector<TH1D*>, m_kNGroups>& hdedxhit);

    /**
    * function to draw dedx dist. for Inner/outer layer
    */
    void plotLayerDist(std::array<TH1D*, m_kNGroups>& hdedxlay);

    /**
    * function to draw pt vs costh and entrance angle distribution for Inner/Outer layer
    */
    void plotQaPars(std::array<TH1D*, m_kNGroups>& hentalay, TH2D* hptcosth);

    /**
    * function to draw symm/Var layer constant
    */
    void plotRelConst(std::vector<double>tempconst, std::vector<double>layerconst, int il);

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
    * 1D cell algorithm
    */
    virtual EResult calibrate() override;


  private:

    double m_eaMin; /**< lower edge of entrance angle */
    double m_eaMax; /**< upper edge of entrance angle */
    double m_eaBW; /**< binwdith of entrance angle bin */
    int m_eaBin; /**< # of bins for entrance angle */
    int m_eaB; /**< reset # of bins for entrance angle for each experiment */

    double m_dedxMin;  /**< lower edge of dedxhit */
    double m_dedxMax;  /**< upper edge of dedxhit */
    int m_dedxBin;  /**< # of bins for dedxhit range */

    double m_ptMax; /**< a limit on transverse momentum  */
    double m_cosMax; /**< a limit on cos theta  */

    double m_truncMin; /**< lower threshold on truncation*/
    double m_truncMax; /**< upper threshold on truncation */

    int m_binSplit;/**< multiply nbins by this factor in full range */

    double m_chargeType; /**< charge type for baseline adj */
    double m_adjustFac;/**< factor with that one what to adjust baseline */

    bool isFixTrunc; /**< true = fix window for all out/inner layers */
    bool isVarBins;  /**< true: if variable bin size is requested */
    bool isRotSymm; /**< if rotation symmetry requested */
    bool isMakePlots; /**< produce plots for status */
    bool isPrintLog; /**< print more debug information */
    bool isMerge; /**< print more debug information */

    std::string m_suffix; /**< add suffix to all plot name  */
    std::string m_runExp; /**< add run and exp to title of plot  */
    std::string m_label[m_kNGroups] = {"SL0", "SL1", "SL2-8"}; /**< add inner/outer superlayer label */

    std::vector<int> m_eaBinLocal; /**< # of var bins for enta angle */
    std::array<std::vector<int>, m_kNGroups> m_binIndex; /**< symm/Var bin numbers */
    std::array<std::vector<double>, m_kNGroups>m_binValue; /**< enta Var bin values */

    std::vector<std::vector<double>> m_onedcors; /**< final vectors of calibration  */

    DBObjPtr<CDCDedx1DCell> m_DBOneDCell; /**< One cell correction DB object */

  };
} // namespace Belle2
