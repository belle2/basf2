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
#include <TH1I.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include<TGraphErrors.h>

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <reconstruction/utility/CDCDedxMeanPred.h>
#include <reconstruction/utility/CDCDedxSigmaPred.h>
#include <reconstruction/utility/CDCDedxHadSat.h>
#include <framework/gearbox/Const.h>

#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/calibration/HadronSaturation.h>

enum gstatus {OK, Failed};

namespace Belle2 {
  class HadronPrep {


  public:

    void prepareSample(std::shared_ptr<TTree> hadron, TFile*& outfile, std::string suffix,
                       std::string pdg, bool ismakePlots, bool correct);
    void defineHisto(std::vector<TH1F*>& htemp, std::string title, std::string pdg);
    void fillHisto(std::shared_ptr<TTree> hadron, std::string pdg, bool correct);

    void plotDist(std::map<int, std::vector<TH1F*>> hist, std::string sname, std::string pdg);
    void plotDist(std::vector<TH2F*> hist, std::string sname, std::string pdg);
    void plotDist(std::vector<TH1F*> hist, std::string sname, std::string pdg);


    void setPars(TFile*& outfile, std::string pdg);
    void fitGaussianWRange(TH1F*& temphist, gstatus& status, double sigmaR);
    void plotGraph(std::string sname, std::string pdg);

    void fit(TH1F*& hist, std::string pdg);


    /**
        * function to delete histograms for dedx and time dist. (in time bins)
        */
    void deleteHisto();

  private:

    std::map<int, std::vector<TH1F*>>  hdedx_bgcosth, hchi_bgcosth;
    std::vector<TH2F*> hdedxvscosth_bg;
    std::vector<TH1F*> hdedx_costh;

    std::map<int, std::vector<double>> sumcos, sumbg, sumres_square;
    std::map<int, std::vector<int>> sumsize;
    std::map<int, std::vector<double>> means, errors;
    double dedxmax = 0.0, dedxmin = 99999999.0;

    int m_bgBins; /**< bins for dedx histogram */
    double m_bgMin; /**< min range of dedx */
    double m_bgMax; /**< max range of dedx */

    int m_cosBins = 8; /**< bins for dedx histogram */
    double m_cosMin = 0; /**< min range of dedx */
    double m_cosMax = 0.95; /**< max range of dedx */

    int m_chiBins; /**< bins for chi histogram */
    double m_chiMin; /**< min range of chi */
    double m_chiMax; /**< max range of chi */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor;

  };
} // namespace Belle2