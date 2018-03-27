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
    m_h2[i] = new TH2D(Form("board_%d", i), Form("board_%d", i), 20, 0., 500, 60, -30, 30);
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
    const double xmax = halfCSize[lay] - 0.1;
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

  TF1* func = new TF1("func", "[0]+[1]/sqrt(x)", 0, 500);
  func->SetParameters(-4, 28);
  for (int ib = 1; ib < 300; ++ib) {
    B2DEBUG(199, "Board ID:" << ib);
    m_h2[ib]->SetDirectory(0);
    if (m_h2[ib]->GetEntries() < 500) {
      m_tw[ib] = 0;
      continue;
    }
    doSliceFitY(ib, 20);
    //    m_h1[ib] = (TH1D*)SliceFit::doSliceFitY(m_h2[ib], 20)->Clone();
    if (!m_h1[ib]) {
      m_tw[ib] = 0;
      continue;
    }
    m_h1[ib]->SetDirectory(0);
    if (m_h1[ib]->GetEntries() < 5) {
      m_tw[ib] = 0;
      continue;
    }
    B2DEBUG(199, "Number of point" << m_h1[ib]->GetEntries());
    m_h1[ib]->Fit("func", "MQ", "", 20, 150);
    m_tw[ib] = func->GetParameter(1);
    B2DEBUG(199, "Prob of fitting:" << func->GetProb());
    B2DEBUG(199, "Fitting Param 0-1:" << func->GetParameter(0) << " - " << func->GetParameter(1));
  }

  //Write histogram to file
  if (m_storeHisto) {
    B2INFO("Storing histogram");
    TFile* fhist = new TFile("histTw.root", "recreate");
    TDirectory* old = gDirectory;
    TDirectory* h1D = old->mkdir("h1D");
    TDirectory* h2D = old->mkdir("h2D");
    h1D->cd();
    for (int ib = 1; ib < 300; ++ib) {
      if (m_h1[ib] == nullptr) continue;
      if (m_h1[ib]->GetEntries() < 5) continue;
      m_h1[ib]->SetMinimum(-5);
      m_h1[ib]->SetMaximum(5);
      m_h1[ib]->Write();
    }

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
  return c_OK;
}
void TimeWalkCalibrationAlgorithm::write()
{
  B2INFO("Save to the local DB");
  CDCTimeWalks* dbTw = new CDCTimeWalks();
  for (int ib = 0; ib < 300; ++ib) {
    dbTw->setTimeWalkParam(ib, m_twPost[ib] + m_tw[ib]);
  }

  if (m_textOutput == true) {
    dbTw->outputToFile(m_outputFileName);
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
  const int nEntries = dbTw->getEntries();
  for (int ib = 0; ib < nEntries; ++ib) {
    m_twPost[ib] = dbTw->getTimeWalkParam(ib);
  }
}
void TimeWalkCalibrationAlgorithm::doSliceFitY(int boardId, int minHitCut = 0)
{
  gPrintViaErrorHandler = kTRUE;
  gErrorIgnoreLevel = 3001;
  auto h2 = m_h2[boardId];
  TString hist_name = h2->GetName();
  double ub = h2->GetYaxis()->GetXmax();
  double lb = h2->GetYaxis()->GetXmin();
  B2DEBUG(199, "Axis: " << lb << "  " << ub);
  if ((h2->GetEntries() / h2->GetNbinsX()) < 30) {
    B2WARNING("Low statictic: " << h2->GetEntries() << " Hits");
    h2->Rebin2D(2, 2, hist_name);
  }

  B2DEBUG(199, "Slice fit for histo " << hist_name);
  B2DEBUG(199, "Number of entries: " << h2->GetEntries());
  TF1* g1 = new TF1("g1", "gaus", lb, ub);
  h2->FitSlicesY(0, 0, -1, minHitCut);
  TString m_name = hist_name + "_1";
  TH1D* hm = (TH1D*)gDirectory->Get(m_name)->Clone("hm");
  if (!hm) {
    B2ERROR(" hist hm doesn't exist");
  }

  B2DEBUG(199, "Number of entries: " << hm->GetEntries());
  m_h1[boardId] = (TH1D*) hm->Clone("hlast");
  TH1D* hlast = m_h1[boardId];
  hlast->Reset();
  hlast->SetName(m_name);
  for (int i = 1; i < h2->GetNbinsX(); ++i) {
    double sum = 0;
    double err = 0;
    double mean = -99;
    TH1D* h1d = h2->ProjectionY("h1d", i, i);
    if (!h1d) continue;
    sum = h1d->GetEntries();
    if (sum < minHitCut) continue; //skip low data slice
    mean = h1d->GetMean();
    double sg = h1d->GetRMS();
    double max = h1d->GetMaximum();
    g1->SetParameters(max, mean, sg);
    h1d->Fit("g1", "QNR", "");
    mean = g1->GetParameter(1);
    err = g1->GetParError(1);
    if (sum > 50) {
      double sg2 = g1->GetParameter(2);
      h1d->Fit("g1", "Q0", "", mean - 1.1 * sg2, mean + 1.1 * sg2);
      mean = g1->GetParameter(1);
      //  err=g1->GetParError(1);
    }
    hlast->SetBinContent(i, mean);
    hlast->SetBinError(i, err);
    h1d->Delete();
  }
}
