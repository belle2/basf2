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
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include <TGraphErrors.h>
#include "TTree.h"

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <reconstruction/utility/CDCDedxMeanPred.h>
#include <reconstruction/utility/CDCDedxSigmaPred.h>
#include <reconstruction/utility/CDCDedxHadSat.h>

#include <framework/gearbox/Const.h>

enum gstatus {OK, Failed};

namespace Belle2 {
  class HadronBgPrep {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    HadronBgPrep();

    /**
    * Destructor
    */
    virtual ~HadronBgPrep() {};

    /**
    * Constructor: set the input variables
    */
    HadronBgPrep(int bgbins, double upperbg, double lowerbg, int cosbins, double uppercos, double lowercos, int injbins,
                 double lowerinj, double upperinj, int nhitbins, double lowernhit, double uppernhit, double cut);

    /**
    * function to prepare sample for monitoring plots, bg curve fitting and sigma vs ionz fitting
    */
    void prepareSample(std::shared_ptr<TTree> hadron, TFile*& outfile, std::string suffix, std::string bgcurvefile,
                       std::string bgsigmafile, std::string pdg, bool ismakePlots);

    /**
    * function to define histograms
    */
    void defineHisto(std::vector<TH1F*>& htemp, std::string svar, std::string stype, std::string pdg);

    /**
    * function to plot the map of histograms
    */
    void plotDist(std::map<int, std::vector<TH1F*>>& hist, std::string suffix, int bins);

    /**
    * function to plot the histograms
    */
    void plotDist(std::vector<TH1F*>& hist, std::string suffix, int nbins);

    /**
    * function to fill the parameters like mean and reso in the tree
    */
    void setPars(TFile*& outfile, std::string& pdg, std::vector<TH1F*>& hdedx_bg, std::vector<TH1F*>& hchi_bg,
                 std::vector<TH1F*>& hionzsigma_bg, std::map<int, std::vector<TH1F*>>& hchi_inj);

    /**
    * function to perform gauss fit for input histogram
    */
    void fitGaussianWRange(TH1F*& temphist, gstatus& status, double sigmaR);

    /**
    * function to fit the histograms
    */
    void fit(TH1F*& hist, std::string pdg);

    /**
    * function to draw the dedx vs costh histograms
    */
    void printCanvasCos(std::map<int, std::vector<TH1F*>>& hchicos_allbg, std::map<int, std::vector<TH1F*>>& hchicos_1by3bg,
                        std::map<int, std::vector<TH1F*>>& hchicos_2by3bg, std::map<int, std::vector<TH1F*>>& hchicos_3by3bg, std::string particle,
                        std::string suffix);

    /**
    * function to set graph cosmetics
    */
    void FormatGraph(TGraphErrors& gr, int flag, std::string name = "")
    {
      if (flag == 0) {
        gr.SetTitle(Form("%s;cos(#theta);#chi_{mean}", name.data()));
        gr.SetMarkerStyle(24);
        gr.SetMarkerColor(2);
        gr.SetMarkerSize(.7);
        gr.SetMaximum(1.0);
        gr.SetMinimum(-1.0);
        gr.GetXaxis()->SetRangeUser(-1, 1);
      } else if (flag == 1) {
        gr.SetMarkerStyle(22);
        gr.SetMarkerColor(9);
        gr.SetMarkerSize(.7);
      } else if (flag == 2) {
        gr.SetTitle(Form("%s;cos(#theta);#sigma", name.data()));
        gr.SetMarkerStyle(24);
        gr.SetMarkerColor(2);
        gr.SetMarkerSize(.7);
        gr.SetMaximum(2);
        gr.SetMinimum(0);
        gr.GetXaxis()->SetRangeUser(-1, 1);
      }
    }

    /**
    * function to clear the variables
    */
    void clearVars()
    {
      m_sumcos.clear();
      m_sumbg.clear();
      m_sumres_square.clear();
      m_sumsize.clear();
      m_means.clear();
      m_errors.clear();
      m_suminj.clear();
    }

    /**
    * function to delete the histograms
    */
    void deleteHistos(std::vector<TH1F*>& htemp, int bins)
    {
      for (int j = 0; j < bins; ++j) delete htemp[j];
    }

    /**
    * function to get the particle mass
    */
    double getParticleMass(std::string particle)
    {
      double mass;
      if (particle == "pion") mass = Const::pion.getMass();
      else if (particle == "kaon") mass = Const::kaon.getMass();
      else if (particle == "proton") mass = Const::proton.getMass();
      else if (particle == "muon") mass = Const::muon.getMass();
      else if (particle == "electron") mass = Const::electron.getMass();
      return mass;
    }

  private:

    std::vector<double> m_sumcos, m_sumbg, m_sumres_square, m_suminj, m_means, m_errors; /**< variables for bg  */
    std::vector<int> m_sumsize, m_injsize; /**< variables for injection time  */

    int m_bgBins; /**< bins for bg  */
    double m_bgMin; /**< min range of bg */
    double m_bgMax; /**< max range of bg */

    int m_injBins; /**< bins for injection time  */
    double m_injMin; /**< min range of injection time */
    double m_injMax; /**< max range of injection time */

    int m_cosBins; /**< bins for cosine */
    double m_cosMin; /**< min range of cosine */
    double m_cosMax; /**< max range of cosine */

    int m_nhitBins; /**< bins for nhits */
    double m_nhitMin; /**< min range of nhits */
    double m_nhitMax; /**< max range of nhits */

    double m_cut;  /**< cut to clean protons */

  };
} // namespace Belle2