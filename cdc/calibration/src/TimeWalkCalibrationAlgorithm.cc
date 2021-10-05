/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/calibration/TimeWalkCalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

#include <TF1.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TTree.h>
#include <TStopwatch.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

TimeWalkCalibrationAlgorithm::TimeWalkCalibrationAlgorithm() : CalibrationAlgorithm("CDCCalibrationCollector")
{
  setDescription(
    " -------------------------- Time Walk Calibration Algorithm -------------------------\n"
  );
}

void TimeWalkCalibrationAlgorithm::createHisto()
{
  B2INFO("Creating and filling histograms");

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  double halfCSize[56];
  for (int i = 0; i < 56; ++i) {
    double R = cdcgeo.senseWireR(i);
    double nW = cdcgeo.nWiresInLayer(i);
    halfCSize[i] = M_PI * R / nW;
  }

  // Histogram for each board
  for (int i = 0; i < 300; ++i) {
    m_h2[i] = new TH2D(Form("board_%d", i), Form("board_%d", i), 50, 0., 500, 60, -30, 30);
  }

  // Read data

  auto tree = getObjectPtr<TTree>("tree");

  Float_t x;
  Float_t t_mea;
  Float_t t_fit;
  Float_t ndf;
  Float_t Pval;
  UShort_t adc;
  UShort_t IWire;
  UChar_t lay;

  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("IWire", &IWire);
  tree->SetBranchAddress("x_u", &x);
  tree->SetBranchAddress("t", &t_mea);
  tree->SetBranchAddress("t_fit", &t_fit);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("adc", &adc);

  /* Disable unused branch */
  std::vector<TString> list_vars = {"lay", "IWire", "x_u", "t", "t_fit", "Pval", "ndf", "adc"};
  tree->SetBranchStatus("*", 0);

  for (TString brname : list_vars) {
    tree->SetBranchStatus(brname, 1);
  }

  const Long64_t nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  TStopwatch time;
  time.Start();
  for (Long64_t i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    const double xmax = halfCSize[lay] - 0.12;
    if ((fabs(x) < m_xmin) || (fabs(x) > xmax)
        || (ndf < m_minNdf)
        || (Pval < m_minPval)) continue; /*select good region*/

    m_h2[cdcgeo.getBoardID(WireID(lay, IWire))]->Fill(adc, fabs(t_mea) - fabs(t_fit));
  }
  time.Stop();
  B2INFO("Time to fill histograms: " << time.RealTime() << "s");

}

CalibrationAlgorithm::EResult TimeWalkCalibrationAlgorithm::calibrate()
{
  B2INFO("Start calibration");
  gROOT->SetBatch(1);

  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);
  B2INFO("Creating CDCGeometryPar object");
  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  prepare();
  createHisto();

  TF1* fold = nullptr;
  if (m_twParamMode == 0)
    fold = new TF1("fold", "[0]/sqrt(x)", 0, 500);
  else if (m_twParamMode == 1)
    fold = new TF1("fold", "[0]*exp(-1*[1]*x)", 0, 500);

  if (fold == nullptr) {
    B2FATAL("Old fitting function is not defined.");
  }


  B2INFO("time walk formular: ");
  [](TF1 * f) { auto title = f->GetTitle(); B2INFO("Expression : " << title);}(fold);
  //  B2INFO("New time walk mode : " << m_twParamMode_new << " with " << m_nTwParams_new << " parameters");

  for (int ib = 1; ib < 300; ++ib) {
    m_flag[ib] = 1;
    B2DEBUG(21, "Board ID:" << ib);
    m_h2[ib]->SetDirectory(0);

    // Ignore if histogram has low stat. (<500 entries)
    if (m_h2[ib]->GetEntries() < 500) {
      m_flag[ib] = 0;
      continue;
    }
    m_h2[ib]->FitSlicesY(0, 1, -1, 10);
    TString name = m_h2[ib]->GetName();
    TString hm_name = name + "_1";
    m_h1[ib] = (TH1D*)gDirectory->Get(hm_name);
    if (!m_h1[ib]) {m_flag[ib] = 0; continue;}
    m_h1[ib]->SetDirectory(0);
    if (m_h1[ib]->GetEntries() < 5) {
      m_flag[ib] = 0;
      B2WARNING("Low statistic, number of points after slice fit: " << m_h1[ib]->GetEntries());
      continue;
    }

    // Add previous correction to this
    for (int p = 0; p < m_nTwParams; ++p) {
      fold->SetParameter(p, m_tw_old[ib][p]);
    }


    if (m_twParamMode == 0) {
      TF1* func = new TF1("func", "[0]+[1]/sqrt(x)", 0, 500);
      func->SetParameters(-4, 28);
      m_h1[ib]->Fit("func", "MQ", "", 20, 150);
    } else if (m_twParamMode == 1) {
      m_h1[ib]->Add(fold);
      fitToExponentialFunc(m_h1[ib]);
    } else {
      B2FATAL("Mode " << m_twParamMode << " is not available, please check again");
    }

    // Read fitted parameters
    TF1* f1 = m_h1[ib]->GetFunction("func");
    if (!f1) { m_flag[ib] = 0; continue;}
    m_constTerm[ib] = f1->GetParameter(0);
    for (int i = 1; i <= m_nTwParams; ++i) {
      m_tw_new[ib][i - 1] = f1->GetParameter(i);
    }


    B2DEBUG(21, "Prob of fitting:" << f1->GetProb());
    B2DEBUG(21, "Fitting Param 0-1:" << f1->GetParameter(0) << " - " << f1->GetParameter(1));

  }

  //Write histogram to file
  if (m_storeHisto) {
    storeHist();
  }

  write();
  updateT0();

  return checkConvergence();
}

void TimeWalkCalibrationAlgorithm::storeHist()
{
  B2INFO("Storing histogram");
  B2DEBUG(21, "Store 1D histogram");
  TFile*  fhist = new TFile(m_histName.c_str(), "RECREATE");
  auto hNDF =   getObjectPtr<TH1F>("hNDF");
  auto hPval =   getObjectPtr<TH1F>("hPval");
  auto hEvtT0 =   getObjectPtr<TH1F>("hEventT0");
  //store NDF, P-val. EventT0 histogram for monitoring during calibration
  if (hNDF && hPval && hEvtT0) {
    hEvtT0->Write();
    hPval->Write();
    hNDF->Write();
  }

  TDirectory* old = gDirectory;
  TDirectory* h1D = old->mkdir("h1D");
  TDirectory* h2D = old->mkdir("h2D");
  h1D->cd();
  for (int ib = 1; ib < 300; ++ib) {
    if (!m_h1[ib] || m_flag[ib] != 1) continue;
    if (m_h1[ib]->GetEntries() < 5) continue;
    m_h1[ib]->SetMinimum(-5);
    m_h1[ib]->SetMaximum(15);
    m_h1[ib]->Write();
  }

  B2DEBUG(21, "Store 2D histogram");
  h2D->cd();
  for (int ib = 1; ib < 300; ++ib) {
    if (m_h2[ib] == nullptr) continue;
    if (m_h2[ib]->GetEntries() < 5) continue;
    m_h2[ib]->Write();

  }

  fhist->Close();
  B2INFO("Hitograms were stored");
}

CalibrationAlgorithm::EResult TimeWalkCalibrationAlgorithm::checkConvergence()
{
  TH1F* hDtw = new TH1F("hDtw", "", 100, -1, 1);
  for (int ib = 0; ib < 300; ++ib) {
    float dtw = (m_tw_new[ib][0] - m_tw_old[ib][0]) / m_tw_old[ib][0];
    if (std::isnan(dtw) == 0) {
      hDtw->Fill(dtw);
    }
  }

  B2INFO(hDtw->GetRMS());

  if (hDtw->GetRMS() < 0.02) {
    return c_OK;
  } else {
    return c_Iterate;
  }
  delete hDtw;
}

void TimeWalkCalibrationAlgorithm::write()
{
  B2INFO("Save to the local DB");
  CDCTimeWalks* dbTw = new CDCTimeWalks();
  int nfailure = 0;

  dbTw->setTwParamMode(m_twParamMode);
  for (int ib = 0; ib < 300; ++ib) {
    if (m_flag[ib] != 1) {
      nfailure += 1;
    }
    if (m_twParamMode == 0) {
      const int num = static_cast<int>(m_tw_old[ib].size());
      for (int i = 0; i < num; ++i) {
        m_tw_new[ib][i] += m_tw_old[ib][i];
      }
    }

    dbTw->setTimeWalkParams(ib, m_tw_new[ib]);
  }

  if (m_textOutput == true) {
    dbTw->outputToFile(m_outputTWFileName);
    B2RESULT("Time-walk coeff. table has been written to " << m_outputTWFileName.c_str());
  }

  saveCalibration(dbTw, "CDCTimeWalks");
  B2RESULT("Failure to calibrate time-walk for " << nfailure << " board");

}

void TimeWalkCalibrationAlgorithm::prepare()
{
  B2INFO("Prepare calibration");

  DBObjPtr<CDCTimeWalks> dbTw;
  m_twParamMode = dbTw->getTwParamMode();

  if (!(m_twParamMode == 0 || m_twParamMode == 1)) {
    B2FATAL("Mode " << m_twParamMode << " haven't implemented yet.");
  }

  B2INFO("tw param mode " << m_twParamMode);
  const int nEntries = dbTw->getEntries();
  for (int ib = 0; ib < nEntries; ++ib) {
    m_tw_old[ib] = dbTw->getTimeWalkParams(ib);
    m_tw_new[ib].resize(m_nTwParams, 0.0);
  }
}

void TimeWalkCalibrationAlgorithm::updateT0()
{
  B2INFO("Add constant term into T0 database");
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  CDCTimeZeros* tz = new CDCTimeZeros();
  double T0;
  for (int ilay = 0; ilay < 56; ++ilay) {
    const unsigned int nW = cdcgeo.nWiresInLayer(ilay);
    for (unsigned int iwire = 0; iwire < nW; ++iwire) {
      WireID wireid(ilay, iwire);
      int bID = cdcgeo.getBoardID(wireid);
      T0 = cdcgeo.getT0(wireid);
      if (m_flag[bID] == 1) {
        tz->setT0(wireid, T0 - m_constTerm[bID]);
      } else {
        tz->setT0(wireid, T0);
      }
    }
  }
  if (m_textOutput == true) {
    tz->outputToFile(m_outputT0FileName);
  }
  saveCalibration(tz, "CDCTimeZeros");
}

void TimeWalkCalibrationAlgorithm::fitToExponentialFunc(TH1D* h1)
{
  h1->SetDirectory(0);
  int max = h1->GetMaximumBin();
  double maxX = h1->GetBinCenter(max);
  double maxY = h1->GetBinContent(max);
  B2DEBUG(21, "Max: id - x - y : " << max << "  " << maxX << "  " << maxY);

  //search for p0
  double p0 = -1;
  h1->Fit("pol0", "MQE", "", maxX + 125, 500);
  if (h1->GetFunction("pol0")) {
    p0 = h1->GetFunction("pol0")->GetParameter(0);
  }
  //create histo = old-p0;
  // fit with expo function to find intial parameters
  TH1D* hshift  = new TH1D("hshift", "shift", h1->GetNbinsX(), 0, 500);
  hshift->SetDirectory(0);
  for (int i = 0; i <= h1->GetNbinsX(); ++i) {
    hshift->SetBinContent(i, h1->GetBinContent(i) - p0);
  }
  hshift->Fit("expo", "MQE", "", 0, 400);
  double p1 = maxY + p0;
  double p2 = -0.04;
  if (hshift->GetFunction("expo")) {
    p1 =  exp(hshift->GetFunction("expo")->GetParameter(0));
    p2 =  hshift->GetFunction("expo")->GetParameter(1);
  }

  // fit with function
  TF1* func = new TF1("func", "[0]+ [1]*exp(-1*[2]*x)", 0, 500);
  func->SetParameters(p0, p1, -1 * p2);
  func->SetParLimits(0, -5, 5);
  func->SetParLimits(1, -5, 500);
  func->SetParLimits(2, -0.01, 0.1);
  h1->Fit("func", "MQE", "", 10, 400);
}
