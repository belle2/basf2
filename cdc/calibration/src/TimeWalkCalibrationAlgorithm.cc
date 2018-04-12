/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Makoto Uchida                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/calibration/TimeWalkCalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TTree.h>
#include "iostream"
#include "string"
#include <framework/utilities/FileSystem.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
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

  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
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

  double x;
  double t_mea;
  double w;
  double t_fit;
  double ndf;
  double Pval;
  unsigned short adc;
  int IWire;
  int lay;

  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("IWire", &IWire);
  tree->SetBranchAddress("x_u", &x);
  tree->SetBranchAddress("t", &t_mea);
  tree->SetBranchAddress("t_fit", &t_fit);
  tree->SetBranchAddress("weight", &w);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("adc", &adc);

  const int nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    const double xmax = halfCSize[lay] - 0.12;
    if ((fabs(x) < m_xmin) || (fabs(x) > xmax)
        || (ndf < m_minNdf)
        || (Pval < m_minPval)) continue; /*select good region*/

    m_h2[cdcgeo.getBoardID(WireID(lay, IWire))]->Fill(adc, fabs(t_mea) - fabs(t_fit));
  }
  B2INFO("Finish making histogram for all channels");
}

CalibrationAlgorithm::EResult TimeWalkCalibrationAlgorithm::calibrate()
{
  B2INFO("Start calibration");
  gROOT->SetBatch(1);

  // We create an EventMetaData object. But since it's possible we're re-running this algorithm inside a process
  // that has already created a DataStore, we need to check if it's already valid, or if it needs registering.
  StoreObjPtr<EventMetaData> evtPtr;
  if (!evtPtr.isValid()) {
    // Construct an EventMetaData object in the Datastore so that the DB objects in CDCGeometryPar can work
    DataStore::Instance().setInitializeActive(true);
    B2INFO("Registering EventMetaData object in DataStore");
    evtPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    B2INFO("Creating EventMetaData object");
    evtPtr.create();
  } else {
    B2INFO("A valid EventMetaData object already exists.");
  }
  // Construct a CDCGeometryPar object which will update to the correct DB values when we change the EventMetaData and update
  // the Database instance
  DBObjPtr<CDCGeometry> cdcGeometry;
  CDC::CDCGeometryPar::Instance(&(*cdcGeometry));
  B2INFO("ExpRun at init : " << evtPtr->getExperiment() << " " << evtPtr->getRun());

  prepare(evtPtr);

  createHisto();

  TF1* fold;
  if (m_twParamMode_old == 0)
    fold = new TF1("fold", "[0]/sqrt(x)", 0, 500);
  else if (m_twParamMode_old == 1)
    fold = new TF1("fold", "[0]*exp(-1*[1]*x)", 0, 500);
  else
    B2FATAL("Mode " << m_twParamMode_old << " haven't implemented yet.");

  B2INFO("Old time walk formular: ");
  fold->Print();
  B2INFO("New time walk mode : " << m_twParamMode_new << " with " << m_nTwParams_new << " parameters");

  m_tw_new[0].resize(m_nTwParams_new, 0.0); //for board 0, no available
  for (int ib = 1; ib < 300; ++ib) {
    m_flag[ib] = 1;
    B2DEBUG(199, "Board ID:" << ib);
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
    for (int p = 0; p < m_nTwParams_old; ++p) {
      fold->SetParameter(p, m_tw_old[ib][p]);
    }
    m_h1[ib]->Add(fold);
    if (m_twParamMode_new == 0) {
      TF1* func = new TF1("func", "[0]+[1]/sqrt(x)", 0, 500);
      func->SetParameters(-4, 28);
      m_h1[ib]->Fit("func", "MQ", "", 20, 150);
    } else if (m_twParamMode_new == 1) {
      fitToExponentialFunc(m_h1[ib]);
    } else {
      B2FATAL("Mode " << m_twParamMode_new << " is not available, please check again");
    }

    // Read fitted parameters
    TF1* f1 = m_h1[ib]->GetFunction("func");
    if (!f1) { m_flag[ib] = 0; continue;}
    m_constTerm[ib] = f1->GetParameter(0);
    m_tw_new[ib].resize(m_nTwParams_new, 0.0);
    for (int i = 1; i <= m_nTwParams_new; ++i) {
      m_tw_new[ib][i - 1] = f1->GetParameter(i);
    }

    B2DEBUG(199, "Prob of fitting:" << f1->GetProb());
    B2DEBUG(199, "Fitting Param 0-1:" << f1->GetParameter(0) << " - " << f1->GetParameter(1));

  }

  //Write histogram to file
  if (m_storeHisto) {
    B2INFO("Storing histogram");

    B2DEBUG(199, "Store 1D histogram");
    TFile* fhist = new TFile("histTw.root", "recreate");
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

    B2DEBUG(199, "Store 2D histogram");
    h2D->cd();
    for (int ib = 1; ib < 300; ++ib) {
      if (m_h2[ib] == nullptr) continue;
      if (m_h2[ib]->GetEntries() < 5) continue;
      m_h2[ib]->Write();

    }

    fhist->Close();
    B2INFO("Hitograms were stored");
  }

  write();
  updateT0();
  return c_OK;
}
void TimeWalkCalibrationAlgorithm::write()
{
  B2INFO("Save to the local DB");
  CDCTimeWalks* dbTw = new CDCTimeWalks();
  for (int ib = 0; ib < 300; ++ib) {
    //temp    dbTw->setTimeWalkParam(ib, m_twPost[ib] + m_tw[ib]);
  }

  if (m_textOutput == true) {
    dbTw->outputToFile(m_outputTWFileName);
  }

  saveCalibration(dbTw, "CDCTimeWalks");
}

void TimeWalkCalibrationAlgorithm::prepare(StoreObjPtr<EventMetaData>& evtPtr)
{
  B2INFO("Prepare calibration");

  const auto exprun =  getRunList();
  B2INFO("Changed ExpRun to: " << exprun[0].first << " " << exprun[0].second);
  evtPtr->setExperiment(exprun[0].first);
  evtPtr->setRun(exprun[0].second);
  DBStore::Instance().update();

  DBObjPtr<CDCTimeWalks> dbTw;
  DBStore::Instance().update();
  m_twParamMode_old = dbTw->getTwParamMode();
  const int nEntries = dbTw->getEntries();
  for (int ib = 0; ib < nEntries; ++ib) {
    m_tw_old[ib] = dbTw->getTimeWalkParams(ib);
  }
}
void TimeWalkCalibrationAlgorithm::updateT0()
{
  B2INFO("Add constant term into T0 database");
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  CDCTimeZeros* tz = new CDCTimeZeros();
  double T0;
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      WireID wireid(ilay, iwire);
      int bID = cdcgeo.getBoardID(wireid);
      if (m_flag[bID] == 1) {
        T0 = cdcgeo.getT0(wireid);
        tz->setT0(wireid, T0 - m_constTerm[bID]);
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
  B2DEBUG(199, "Max: id - x - y : " << max << "  " << maxX << "  " << maxY);

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
