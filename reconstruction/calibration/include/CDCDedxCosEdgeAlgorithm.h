/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TH1D.h>
#include <TPaveText.h>

#include<map>
#include<vector>

#include <reconstruction/dbobjects/CDCDedxCosineEdge.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

enum fitstatus {FitOK, FitFailed, LowStats};

namespace Belle2 {

  /**
  * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
  */
  class CDCDedxCosEdgeAlgorithm : public CalibrationAlgorithm {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxCosEdgeAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxCosEdgeAlgorithm() {}

    /**
    * function to merge paylaods (if relative)
    */
    void setMergePayload(bool value = true) {m_isMerge = value;}

    /**
    * function enable monitoring plots
    */
    void setMonitoringPlots(bool value = false) {m_isMakePlots = value;}

    /**
    * set sigma to restrict fit in (mean+/sigma) range
    */
    void setFitWidth(double value = 2.5) {m_sigLim = value;}

    /**
    * function to set number of cosine bins on (equal posi and negi side)
    */
    void setCosineBins(unsigned int value) {m_npBins = value;}

    /**
    * function to set negative cosine range
    */
    void setNegCosRange(double min, double max) {m_negMin = min; m_negMax = max;}

    /**
    * function to set positive cosine range
    */
    void setPosCosRange(double min, double max) {m_posMin = min; m_posMax = max;}

    /**
    * function to set dedx histogram parameters
    */
    void setDedxHistPars(int value, double min, double max)
    {
      m_dedxBins = value;
      m_dedxMin = min;
      m_dedxMax = max;
    }

    /**
    * funtion to get info about current exp and run
    */
    void getExpRunInfo();

    /**
    * function to perform gaus fit for given histogram
    */
    void fitGaussianWRange(TH1D*& temphist, fitstatus& status);

    /**
    * function to store new payload after full calibration
    */
    void createPayload(std::vector<std::vector<double>>& vfinalconst);

    /**
    * funtion to draw dedx histograms for each bin
    */
    void plotHist(std::vector<TH1D*>& hdedx, std::map<int, std::vector<double>>& fPars, std::string type);

    /**
    * function to draw the fit parameters (relative gains and resolutions)
    */
    void plotFitPars(std::map<int, std::vector<double>>& fPars_Neg, std::map<int, std::vector<double>>& fPars_Pos);

    /**
    * function to draw the final calibration constants and comparison with old constants
    */
    void plotConstants(std::vector<std::vector<double>>& vfinalconst);

    /**
     * function to draw the stats plots
     */
    void plotStats();

    /**
    * function to change text styles
    */
    void setTextCosmetics(TPaveText* pt, double size)
    {
      pt->SetTextAlign(11);
      pt->SetFillStyle(3001);
      pt->SetLineColor(2);
      pt->SetTextFont(82);
      pt->SetTextSize(size);
    }

    /**
     * function to change histogram styles
     */
    void setHistCosmetics(TH1D& hist, Color_t color, double min, double max, double size)
    {
      hist.SetMarkerStyle(20);
      hist.SetMarkerSize(0.60);
      hist.SetMarkerColor(color);
      hist.SetLineColor(color);
      hist.SetStats(0);
      hist.GetYaxis()->SetRangeUser(min, max);
      hist.GetXaxis()->SetLabelSize(size);
      hist.GetYaxis()->SetLabelSize(size);
    }

  protected:

    /**
    * Cosine edge algorithm
    */
    virtual EResult calibrate() override;

  private:

    bool m_isMakePlots; /**< enable saving plots */
    bool m_isMerge; /**< merge payload if calculated relative */

    double m_sigLim; /**< gaussian fit sigma limit */
    unsigned int m_npBins; /**< number of bins across cosine range */
    double m_negMin; /**< min neg cosine angle */
    double m_negMax; /**< max neg cosine angle */
    double m_posMin; /**< min pos cosine angle */
    double m_posMax; /**< max pos cosine angle*/
    int m_dedxBins; /**< number of bins for dedx histogram */
    double m_dedxMin; /**< min dedx range */
    double m_dedxMax; /**< max dedx range */

    std::string m_suffix; /**< suffix for better plot naming */

    DBObjPtr<CDCDedxCosineEdge> m_DBCosineCor; /**< CoseEdge correction DB object */
  };
} // namespace Belle2
