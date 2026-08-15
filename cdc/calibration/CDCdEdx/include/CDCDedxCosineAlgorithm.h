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
#include <TLegend.h>

namespace Belle2 {

  /**
  * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
  */
  class CDCDedxCosineAlgorithm : public CalibrationAlgorithm {

  public:

    static constexpr int m_kNGroups = 3; /**< SL grouping: inner (SL0), middle (SL1), outer (SL2–8) */

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxCosineAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxCosineAlgorithm() {}

    /**
    * function to make flag active for method of sep
    */
    void setMethodSep(bool value = true) {isMethodSep = value;}

    /**
    * function to make flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

    /**
    * function to decide merge vs relative gains
    */
    void setMergePayload(bool value = true) {isMergePayload = value;}

    /**
    * set sigma to restrict fit range around mean
    */
    void setFitWidth(double value = 2.5) {m_sigLim = value;}

    /**
    * function to set number of cosine bins for calibration
    */
    void setCosineBins(unsigned int value = 100) {m_cosBin = value;}

    /**
    * function to set min/max range of cosine for calibration
    */
    void setCosineRange(double min = -1.0, double max = 1.0) {m_cosMin = min; m_cosMax = max;}

    /**
    * function to set nbins of dedx dist for calibration
    */
    void setHistBins(int value = 250) {m_dedxBin = value;}

    /**
    * function to set min/max range of dedx dist for calibration
    */
    void setHistRange(double min = 0.0, double max = 5.0) {m_dedxMin = min; m_dedxMax = max;}

    /**
    * adding suffix to control plots
    */
    void setSuffix(const std::string& value) {m_suffix = value;}

    /**
    * function to extract calibration run/exp
    */
    void getExpRunInfo();

    /**
    * function to define dE/dx histograms
    */
    void defineHisto(std::vector<TH1D*>& hdedx, const std::string& tag, const std::string& chargeLabel);

    /**
    * function to define cosine histograms
    */
    TH1D* defineCosthHist(const std::string& tag);

    /**
    * function to fit histogram in each cosine bin
    */
    void fitGaussianWithRange(TH1D*& temphist, TString& status);

    /**
    * function to store new payload after full calibration
    */
    void createPayload(std::vector<double> cosine);

    /**
    * function to draw the dE/dx histogram in costh bins
    */
    void plotdedxHist(std::vector<TH1D*>& hDedxCos_all, std::vector<TH1D*>& hDedxCos_neg, std::vector<TH1D*>& hDedxCos_pos);

    /**
    * Set basic style (color, marker, title, y-range) for a TH1D histogram.
    */
    void setHist(TH1D* h, int color, const char* title,
                 double ymin, double ymax, int marker = 20)
    {
      h->SetLineColor(color);
      h->SetLineWidth(2);
      h->SetMarkerColor(color);
      h->SetMarkerStyle(marker);
      h->SetMarkerSize(0.7);
      h->SetStats(0);

      if (strlen(title) > 0)
        h->SetTitle(title);

      h->GetYaxis()->SetRangeUser(ymin, ymax);
    }

    /**
    * Plot cos(theta) distributions for all, positive, and negative tracks.
    */
    void plotCosThetaDist(TH1D* hCosth_all, TH1D* hCosth_pos, TH1D* hCosth_neg);

    /**
    * Plot dE/dx fit results for all, negative, and positive tracks.
    */
    void plotFitResults(const std::vector<std::vector<double>>& dedxAll,  const std::vector<std::vector<double>>& dedxNeg,
                        const std::vector<std::vector<double>>& dedxPos);

    /**
    * function to draw the stats plots
    */
    void plotEventStats();

    /**
    * function to draw the old/new final constants
    */
    void plotConstants();

    /**
    * Representative CDC layer for each SL group (used to access group-wise constants):
    * SL0 => 1, SL1 => 9, SL2-8 => 17
    */
    unsigned int getRepresentativeLayer(unsigned int igroup) const
    {
      static const std::array<unsigned int, m_kNGroups> repLayer = {1, 9, 17};
      return repLayer.at(igroup);
    }

  protected:

    /**
    * Cosine algorithm
    */
    virtual EResult calibrate() override;

  private:

    /**
    * @brief Container for Gaussian fit results of a histogram.
    *
    * Stores the extracted fit parameters along with their uncertainties
    * and a status flag indicating the success or failure of the fit.
    */

    struct FitValues {
      double mean = 1.0;  /**< mean      : fitted mean value */
      double meanErr = 0.0;  /**< meanErr   : uncertainty on the mean */
      double sigma = 0.0;  /**< sigma     : fitted width */
      double sigmaErr = 0.0;  /**< sigmaErr  : uncertainty on the width */
      TString status;  /**< status    : fit status flag (e.g. "FitOK", "FitFailed") */
    };

    /**
    * Fit histogram with Gaussian and return mean, error, and width.
    */
    FitValues fitHistogram(TH1D*& hist);

    bool isMethodSep; /**< if e+ e- need to be consider sep */
    bool isMakePlots; /**< produce plots for status */
    bool isMergePayload; /**< merge payload at the time of calibration */
    double m_sigLim; /**< gaussian fit sigma limit */
    unsigned int m_cosBin; /**< number of bins across cosine range */
    double m_cosMin; /**< min cosine angle for cal */
    double m_cosMax; /**< max cosine angle for cal */

    int m_dedxBin; /**< number of bins for dedx histogram */
    double m_dedxMin; /**< min dedx range for gain cal */
    double m_dedxMax; /**< max dedx range for gain cal */

    std::string m_suffix; /**< add suffix to all plot name  */

    std::vector<std::vector<double>> m_coscors; /**< final vectors of calibration  */

    const std::array<std::string, m_kNGroups> m_label = {"SL0", "SL1", "SL2-8"}; /**< add inner/outer superlayer label */

    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
  };
} // namespace Belle2
