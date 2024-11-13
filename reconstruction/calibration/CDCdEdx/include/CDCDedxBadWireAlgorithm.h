/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>

#include <TCanvas.h>
#include <TH2F.h>
#include <TH1I.h>
#include <TMath.h>
#include <TLine.h>
#include <TStyle.h>
#include <TPaveText.h>
#include <TText.h>
#include <TLegend.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <reconstruction/dbobjects/CDCDedxBadWires.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>

#include <calibration/CalibrationAlgorithm.h>

#include <cdc/dbobjects/CDCGeometry.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeometryParConstants.h>


namespace Belle2 {

  /**
   * A calibration algorithm for CDC dE/dx to find the bad wires
   *
   */
  class CDCDedxBadWireAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxBadWireAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxBadWireAlgorithm() {}

    /**
    * function to enable plotting
    */
    void setMonitoringPlots(bool value = false) {m_isMakePlots = value;}

    /**
    * function to set high dedx fraction threshold
    */
    void setHighFracThres(double value) {m_fracThres = value;}

    /**
    * function to set RMS Threshold
    */
    void setRMSThres(double value) {m_rmsThres = value;}

    /**
    * function to set Mean Threshold
    */
    void setMeanThres(double value) {m_meanThres = value;}

    /**
     * function to choose adc or dedx as variable
     */
    void setADC(bool value = false)
    {
      m_isADC = value;
      if (m_isADC) m_varMax = 1000.0;
      else m_varMax = 7.0;
    }

    /**
    * function to set adc/dedx parameters
    */
    void setHistPars(int nbin, double min, double max)
    {
      m_varBins = nbin;
      m_varMin = min;
      m_varMax = max;
    }

    /**
    * function to get extract calibration run/exp
    */
    void getExpRunInfo();

    /**
      * function to draw per wire plots
      */
    void plotWireDist(const std::vector<double>& inwires, std::map<int, std::vector<double>>& vhitvar);

    /**
     * function to print canvas
     */
    void printCanvas(TList* list, TList* hflist, Color_t color);

    /**
     * function to plot wire status map (all, bad and dead)
     */
    void plotBadWireMap(const std::vector<double>& vbadwires, const std::vector<double>& vdeadwires);

    /**
     * function to get wire map with input file (all, bad and dead)
     */
    TH2F* getHistoPattern(const std::vector<double>& inwires, const std::string& suffix, int& total);

    /**
     * function to plot the QA (decision) parameters
     */
    void plotQaPars(std::map<int, std::vector<double>>& qapars);

    /**
     * function to draw the stats
     */
    void plotEventStats();

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
    void setHistCosmetics(TH2F* hist, Color_t color)
    {
      hist->SetMarkerStyle(20);
      hist->SetMarkerSize(0.3);
      hist->SetMarkerColor(color);
      hist->SetFillColor(color);
      hist->SetStats(0);
    }

  protected:

    /**
     * cdcdedx badwire algorithm
     */
    virtual EResult calibrate() override;

  private:

    unsigned int c_nwireCDC; /**< number of wires in CDC */

    bool m_isMakePlots; /**< produce plots for status */
    bool m_isADC; /**< Use adc if(true) else dedx for calibration*/

    int m_varBins; /**< number of bins for input variable */
    double m_varMin; /**< min range for input variable */
    double m_varMax; /**< max range for input variable */
    double m_meanThres; /**< mean Threshold accepted for good wire */
    double m_rmsThres; /**< rms Threshold accepted for good wire */
    double m_fracThres; /**< high-frac Threshold accepted for good wire */
    double m_amean; /**< average mean of dedx for all wires */
    double m_arms; /**< average rms of dedx for all wires */

    std::string m_varName; /**< std::string to set var name (adc or dedx) */
    std::string m_suffix; /**< suffix std::string for naming plots */

    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Badwire DB object */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wiregain DB object */
    DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
  };
} // namespace Belle2
