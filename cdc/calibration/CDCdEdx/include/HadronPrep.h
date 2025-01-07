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
#include <TH2F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include<TGraphErrors.h>

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/utilities/CDCDedxHadSat.h>
#include <framework/gearbox/Const.h>

#include <cdc/calibration/CDCdEdx/HadronBgPrep.h>

namespace Belle2 {

  /**
  * Class to prepare sample for hadron saturation calibration
  */
  class HadronPrep {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    HadronPrep();

    /**
    * Destructor
    */
    virtual ~HadronPrep() {};

    /**
    * Constructor: set the input variables
    */
    HadronPrep(int bgbins, double upperbg, double lowerbg, int cosbins, double uppercos, double lowercos, double cut);

    /**
    * function to prepare sample for monitoring plots, bg curve fitting and sigma vs ionz fitting
    */
    void prepareSample(std::shared_ptr<TTree> hadron, TFile*& outfile, std::string suffix,
                       std::string pdg, bool ismakePlots, bool correct);

    /**
    * function to define histograms
    */
    void defineHisto(std::vector<TH1F*>& htemp, std::string title, std::string pdg);

    /**
    * function to plot the map of histograms
    */
    void plotDist(std::map<int, std::vector<TH1F*>>& hist, std::string sname, std::string pdg);

    /**
    * function to plot the 2-D histograms
    */
    void plotDist(std::vector<TH2F*>& hist, std::string sname, std::string pdg);

    /**
    * function to fill the parameters like mean and reso in the tree
    */
    void setPars(TFile*& outfile, std::map<int, std::vector<TH1F*>>& hdedx_bgcosth, std::string pdg);

    /**
    * function to make graph dedx vs cos in different bg bins
    */
    void plotGraph(std::string sname, std::string pdg);

    /**
    * function to clear the variables
    */
    void clear();

  private:

    std::map<int, std::vector<double>> m_sumcos;  /**< variables to add cos values */
    std::map<int, std::vector<double>> m_sumbg; /**< variables to add bg values */
    std::map<int, std::vector<double>> m_means; /**< mean variable */
    std::map<int, std::vector<double>> m_errors;  /**< error variable  */

    std::map<int, std::vector<int>> m_sumsize; /**< variables for size  */

    double m_dedxmax = 0.0; /**< variables to set maximum dedx mean  */
    double m_dedxmin = 99999999.0; /**< variables to set minimum dedx mean  */

    int m_bgBins; /**< bins for dedx histogram */
    double m_bgMin; /**< min range of dedx */
    double m_bgMax; /**< max range of dedx */

    int m_cosBins; /**< bins for dedx histogram */
    double m_cosMin; /**< min range of dedx */
    double m_cosMax; /**< max range of dedx */

    double m_cut;  /**< cut to clean protons */

  };
} // namespace Belle2