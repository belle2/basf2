/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <fstream>

#include <TH1D.h>
#include <TH1I.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <reconstruction/dbobjects/CDCDedxInjectTime.h>

// namespace constants
namespace numdedx {
  static const int nrings = 2;
  static const int nvectors = 6;
}

enum fstatus {fitOK, fitFailed};

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx injection time (HER/LER)
   */
  class CDCDedxInjectTimeAlgorithm : public CalibrationAlgorithm {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxInjectTimeAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxInjectTimeAlgorithm() {}

    /**
    * function to decide merged vs relative calibration
    */
    void setMergePayload(bool value = true) {m_isMerge = value;}

    /**
    * function to enable monitoring plots
    */
    void setMonitoringPlots(bool value = false) {m_ismakePlots = value;}

    /**
    * function to set fit range (sigma)
    */
    void setFitWidth(double value) {m_sigmaR = value;}

    /**
    * function to set min # of tracks in time bins (0-40ms)
    */
    void setMinTracks(int value) {m_thersE = value;}

    /**
    * function to set dedx hist parameters
    */
    void setDedxPars(int value, double min, double max)
    {
      m_dedxBins = value;
      m_dedxMin = min;
      m_dedxMax = max;
    }

    /**
    * function to perform gaus fit for input histogram
    */
    void fitGaussianWRange(TH1D*& temphist, fstatus& status);

    /**
    * function to get exp/run information (payload object, plotting)
    */
    void getExpRunInfo();

    /**
    * function to set/reset time bins
    */
    void defineTimeBins();

    /**
    * function to define histograms for dedx and time dist. (in time bins)
    */
    void defineHisto(std::array<std::vector<TH1D*>, numdedx::nrings>& htemp,  std::string var);

    /**
    * function to define injection time bins histograms (monitoring only)
    */
    void defineTimeHisto(std::array<std::array<TH1D*, 3>, numdedx::nrings>& htemp);

    /**
    * check statistics for obtaining calibration const.
    */
    void checkStatistics(std::array<std::vector<TH1D*>, numdedx::nrings>& hvar);

    /**
    * function to correct dedx mean/reso and return corrected vector map
    */
    void correctBinBias(std::map<int, std::vector<double>>& varcorr, std::map<int, std::vector<double>>& var,
                        std::map<int, std::vector<double>>& time, TH1D*& htimes);

    /**
    * function to store payloads after full calibration
    */
    void createPayload(std::array<double, numdedx::nrings>& scale);

    /**
    * function to draw event/track statistics plots
    */
    void plotEventStats();

    /**
    * function to draw dedx and time dist.
    */
    void plotBinLevelDist(std::array<std::vector<TH1D*>, numdedx::nrings>& hvar, std::string var);

    /**
    * function to relative constant from dedx fit mean/reso
    */
    void plotRelConstants(std::map<int, std::vector<double>>& meancorr, std::map<int, std::vector<double>>& resocorr,
                          std::map<int, std::vector<double>>& vmeans, std::map<int, std::vector<double>>& vresos,
                          std::array<std::vector<TH1D*>, numdedx::nrings>& htime);

    /**
    * function to final constant from merging or abs fits
    */
    void plotFinalConstants(std::map<int, std::vector<double>>& vmeans, std::map<int, std::vector<double>>& vresos,
                            std::array<double, numdedx::nrings>& scale);

    /**
    * function to injection time distributions (HER/LER in three bins)
    */
    void plotInjectionTime(std::array<std::array<TH1D*, 3>, numdedx::nrings>& hvar);

    /**
    * function to set histogram cosmetics
    */
    void setHistStyle(TH1D*& htemp, const int ic, const int is, const double min, const double max)
    {
      htemp->SetStats(0);
      htemp->LabelsDeflate();
      htemp->SetMarkerColor(ic);
      htemp->SetMarkerStyle(is);
      htemp->GetXaxis()->SetLabelOffset(-0.055);
      htemp->GetYaxis()->SetTitleOffset(0.75);
      htemp->SetMinimum(min);
      htemp->SetMaximum(max);
    }

    /**
    * function to return time label for histograms labeling
    */
    std::string getTimeBinLabel(const double& tedges, const int& it)
    {
      std::string label = "";
      if (tedges < 2e4)label = Form("%0.01f-%0.01fK", m_tedges[it] / 1e3, m_tedges[it + 1] / 1e3);
      else if (tedges < 1e5)label = Form("%0.0f-%0.0fK", m_tedges[it] / 1e3, m_tedges[it + 1] / 1e3);
      else label = Form("%0.01f-%0.01fM", m_tedges[it] / 1e6, m_tedges[it + 1] / 1e6);
      return label;
    }

    /**
    * function to delete histograms for dedx and time dist. (in time bins)
    */
    void deleteHisto(std::array<std::vector<TH1D*>, numdedx::nrings>& htemp)
    {
      for (unsigned int ir = 0; ir < c_rings; ir++) {
        for (unsigned int it = 0; it < m_tbins; it++) {
          delete htemp[ir][it];
        }
      }
    }

    /**
    * function to define injection time bins histograms (monitoring only)
    */
    void deleteTimeHisto(std::array<std::array<TH1D*, 3>, numdedx::nrings>& htemp)
    {
      const int tzoom = 3;
      for (unsigned int ir = 0; ir < c_rings; ir++) {
        for (int wt = 0; wt < tzoom; wt++) {
          delete htemp[ir][wt];
        }
      }
    }

  protected:

    /**
    * CDC dE/dx Injection time algorithm
    */
    virtual EResult calibrate() override;

  private:

    static const int c_rings = numdedx::nrings;  /**< injection ring constants */
    std::array<std::string, numdedx::nrings> m_sring{"ler", "her"};/**< injection ring name */

    std::vector<double> m_vtedges; /**< external time vector*/
    std::vector<double> m_vtlocaledges; /**< internal time vector*/

    double* m_tedges; /**< internal time array (copy of vtlocaledges) */
    unsigned int m_tbins;  /**< internal time bins */
    double m_sigmaR; /**< fit dedx dist in sigma range */

    int m_dedxBins; /**< bins for dedx histogram */
    double m_dedxMin; /**< min range of dedx */
    double m_dedxMax; /**< max range of dedx */

    int m_countR; /**< a hack for running functions once */
    int m_thersE; /**< min tracks to start calibration */

    bool m_isminStat; /**< flag to merge runs for statistics thershold*/
    bool m_ismakePlots; /**< produce plots for monitoring */
    bool m_isMerge; /**< merge payload when rel constant*/

    std::string m_prefix; /**< string prefix for plot names */
    std::string m_suffix; /**< string suffix for object names */

    std::vector<std::vector<double>> m_vinjPayload; /**< vector to store payload values*/

    DBObjPtr<CDCDedxInjectTime> m_DBInjectTime; /**< Injection time DB object */
  };
} // namespace Belle2