/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>

#include <TMath.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TTree.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TRandom.h>


#include <reconstruction/dbobjects/CDCDedx1DCell.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  /**
   * A calibration algorithm for CDC dE/dx electron: 1D enta cleanup correction
   *
   */
  class CDCDedx1DCellAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedx1DCellAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedx1DCellAlgorithm() {}

    /**
    * adding suffix to control plots
    */
    void setSuffix(const std::string& value) {m_suffix = value;}

    /**
    * Set Var bins flag to on or off
    */
    void setVariableBins(bool value) {isVarBins = value;}

    /**
    * funtion to set truncation method (local vs global)
    */
    void setLayerTrunc(bool value = false) {isFixTrunc = value;}

    /**
    * set false if generating absoulte (not relative)
    * payload
    */
    void setMergePayload(bool value) { isMerge = value;}

    /**
    * set bin split factor for all range
    */
    void setSplitFactor(int value) {m_binSplit = value;}

    /**
    * set rotation sys to copy constants from one region to other
    */
    void setRotSymmetry(bool value) {isRotSymm = value;}

    /**
     * function to set bins of trunction from histogram
    */
    void setTrucationBins(double lowedge, double upedge)
    {
      m_truncMin = lowedge; m_truncMax = upedge ;
    }

    /**
    * funtion to set flag active for plotting
    */
    void enableExtraPlots(bool value = false) {isMakePlots = value;}

    /**
    * funtion to set pt limit
    */
    void setPtLimit(double value) {m_ptMax = value;}

    /**
    * funtion to set cos #theta limit
    */
    void setCosLimit(double value) {m_cosMax = value;}

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
    * class funtion to set rotation symmetry
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
    * function to get extract calibration run/exp
    */
    void getExpRunInfo();

    /**
    * class function to create vectors for bin mappping (Var->symm)
    */
    void CreateBinMapping();

    /**
    * function to define histograms
    */
    void defineHisto(std::vector<TH1D*>  hdedxhit[2],  TH1D* hdedxlay[2], TH1D* hentalay[2]);

    /**
    * function to get bins of trunction from histogram
    */
    void getTruncatedBins(TH1D* hist, int& binlow, int& binhigh);

    /**
    * function to get truncated mean
    */
    double getTruncationMean(TH1D* hist, int binlow, int binhigh);

    /**
     * funtion to generate final constants
     */
    void createPayload();

    /**
     * funtion to plot merging factor
     */
    void plotMergeFactor(std::map<int, std::vector<double>> bounds, const std::array<int, 2> nDev,
                         std::map<int, std::vector<int>> steps);

    /**
    * function to draw the dE/dx histrogram in enta bins
    */
    void plotdedxHist(std::vector<TH1D*> hdedxhit[2]);

    /**
    * funtion to draw dedx dist. for Inner/outer layer
    */
    void plotLayerDist(TH1D* hdedxL[2]);

    /**
    * funtion to draw pt vs costh and entrance angle distribution for Inner/Outer layer
    */
    void plotQaPars(TH1D* hentalay[2], TH2D* hptcosth);

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

    double m_eaMin; /**< lower edge of enta angle */
    double m_eaMax; /**< upper edge of enta angle */
    double m_eaBW; /**< binwdith of enta angle bin */
    int m_eaBin; /**< # of bins for etna angle */

    double m_dedxMin;  /**< lower edge of dedxhit */
    double m_dedxMax;  /**< upper edge of dedxhit */
    int m_dedxBin;  /**< # of bins for dedxhit range */

    double m_ptMax; /**< a limit on transverse momentum  */
    double m_cosMax; /**< a limit on cos theta  */

    double m_truncMin; /**< lower thershold on truncation*/
    double m_truncMax; /**< uppr thershold on truncation */

    int m_binSplit;/**< multiply nbins by this factor in full range */
    int m_binMerge;/**< merge bins by this factor in (-pi/2 <-> -pi/4) and (pi/4 <-> pi/2) region*/

    double m_chargeType; /**< charge type for baseline adj */
    double m_adjustFac;/**< faactor with that one what to adjust baseline */

    bool isFixTrunc; /**< true = fix window for all out/inner layers */
    bool isVarBins;  /**< true: if variable bin size is requested */
    bool isRotSymm; /**< if rotation symmtery requested */
    bool isMakePlots; /**< produce plots for status */
    bool isPrintLog; /**< print more debug information */
    bool isMerge; /**< print more debug information */

    std::string m_suffix; /**< add suffix to all plot name  */
    std::string m_runExp; /**< add suffix to all plot name  */
    std::string m_label[2] = {"IL", "OL"}; /**< add inner/outer layer label */

    std::vector<int> m_eaBinLocal; /**< # of var bins for etna angle */
    std::array<std::vector<int>, 2> m_binIndex; /**< symm/Var bin numebrs */
    std::array<std::vector<double>, 2>m_binValue; /**< etna Var bin values */

    std::vector<std::vector<double>> m_onedcors; /**< final vectors of calibration  */

    DBObjPtr<CDCDedx1DCell> m_DBOneDCell; /**< One cell correction DB object */

  };
} // namespace Belle2
