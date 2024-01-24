/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/calibration/T0Correction.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

#include <TError.h>
#include <TROOT.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

Belle2::CDC::T0Correction::T0Correction():
  m_firstExperiment(0), m_firstRun(0),
  m_lastExperiment(-1), m_lastRun(-1)
{
  /*
   setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
    );
  */
}

void Belle2::CDC::T0Correction::CreateHisto()
{

  B2INFO("CreateHisto");
  double x;
  double t_mea;
  double w;
  double t_fit;
  double ndf;
  double Pval;
  int IWire;
  int lay;

  //  auto& tree = getObject<TTree>("cdcCalib");
  TChain* tree = new TChain("tree");
  tree->Add(m_inputRootFileName.c_str());
  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("IWire", &IWire);
  tree->SetBranchAddress("x_u", &x);
  tree->SetBranchAddress("t", &t_mea);
  tree->SetBranchAddress("t_fit", &t_fit);
  tree->SetBranchAddress("weight", &w);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);

  /* Disable unused branch */
  std::vector<TString> list_vars = {"lay", "IWire", "x_u", "t", "t_fit",  "weight", "Pval", "ndf"};
  tree->SetBranchStatus("*", 0);

  for (TString brname : list_vars) {
    tree->SetBranchStatus(brname, 1);
  }


  double halfCSize[56];
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  for (int i = 0; i < 56; ++i) {
    double R = cdcgeo.senseWireR(i);
    double nW = cdcgeo.nWiresInLayer(i);
    halfCSize[i] = M_PI * R / nW;
  }

  m_hTotal = new TH1F("hTotal", "hTotal", 30, -15, 15);
  //for each channel
  for (int il = 0; il < 56; il++) {
    const int nW = cdcgeo.nWiresInLayer(il);
    for (int ic = 0; ic < nW; ++ic) {
      m_h1[il][ic] = new TH1F(Form("h1%d@%d", il, ic), Form("L%d_cell%d", il, ic), 30, -15, 15);
    }
  }
  //for each board
  for (int ib = 0; ib < 300; ++ib) {
    m_hT0b[ib] = new TH1F(Form("hT0b%d", ib), Form("boardID_%d", ib), 100, -20, 20);
  }
  //read data
  const int nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    double xmax = halfCSize[lay] - 0.1;
    if ((fabs(x) < m_xmin) || (fabs(x) > xmax)
        || (ndf < m_ndfmin)
        || (Pval < m_Pvalmin)) continue; /*select good region*/
    //each channel
    m_hTotal->Fill(t_mea - t_fit);
    m_h1[lay][IWire]->Fill(t_mea - t_fit);
    //each board
    m_hT0b[cdcgeo.getBoardID(WireID(lay, IWire))]->Fill(t_mea - t_fit);
  }
  B2INFO("Finish making histogram for all channels");
}

bool Belle2::CDC::T0Correction::calibrate()
{
  B2INFO("Start calibration");

  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;

  CreateHisto();
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  TF1* g1 = new TF1("g1", "gaus", -100, 100);
  vector<double> b, db, Sb, dSb;
  vector<double> c[56];
  vector<double> dc[56];
  vector<double> s[56];
  vector<double> ds[56];

  B2INFO("Gaus fitting for whole channel");
  double par[3];
  m_hTotal->SetDirectory(0);
  double mean = m_hTotal->GetMean();
  m_hTotal->Fit("g1", "Q", "", mean - 15, mean + 15);
  g1->GetParameters(par);

  B2INFO("Gaus fitting for each board");
  for (int ib = 1; ib < 300; ++ib) {
    if (m_hT0b[ib]->GetEntries() < 10) continue;
    mean = m_hT0b[ib]->GetMean();
    m_hT0b[ib]->SetDirectory(0);
    m_hT0b[ib]->Fit("g1", "Q", "", mean - 15, mean + 15);
    g1->GetParameters(par);
    b.push_back(ib);
    db.push_back(0.0);
    Sb.push_back(par[1]);
    dSb.push_back(g1->GetParError(1));
    dtb[ib] = par[1];
    err_dtb[ib] = g1->GetParError(1);
  }
  B2INFO("Gaus fitting for each cell");
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      const int n = m_h1[ilay][iwire]->GetEntries();
      B2DEBUG(99, "layer " << ilay << " wire " << iwire << " entries " << n);
      if (n < 10) continue;
      mean = m_h1[ilay][iwire]->GetMean();
      m_h1[ilay][iwire]->SetDirectory(0);
      m_h1[ilay][iwire]->Fit("g1", "Q", "", mean - 15, mean + 15);
      g1->GetParameters(par);
      c[ilay].push_back(iwire);
      dc[ilay].push_back(0.0);
      s[ilay].push_back(par[1]);
      ds[ilay].push_back(g1->GetParError(1));

      dt[ilay][iwire] = par[1];
      err_dt[ilay][iwire] = g1->GetParError(1);
    }
  }


  if (m_storeHisto) {
    B2INFO("Store histo");
    TFile* fout = new TFile("Correct_T0.root", "RECREATE");
    fout->cd();
    TGraphErrors* gr[56];
    TDirectory* top = gDirectory;
    m_hTotal->Write();
    TDirectory* subDir[56];
    for (int ilay = 0; ilay < 56; ++ilay) {
      subDir[ilay] = top ->mkdir(Form("lay_%d", ilay));
      subDir[ilay]->cd();
      for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
        m_h1[ilay][iwire]->Write();
      }
    }
    top->cd();
    TDirectory* corrT0 = top->mkdir("DeltaT0");
    corrT0->cd();

    if (b.size() > 2) {
      TGraphErrors* grb = new TGraphErrors(b.size(), &b.at(0), &Sb.at(0), &db.at(0), &dSb.at(0));
      grb->SetMarkerColor(2);
      grb->SetMarkerSize(1.0);
      grb->SetTitle("#DeltaT0;BoardID;#DeltaT0[ns]");
      grb->SetMaximum(10);
      grb->SetMinimum(-10);
      grb->SetName("Board");
      grb->Write();
    }
    for (int sl = 0; sl < 56; ++sl) {
      if (c[sl].size() < 2) continue;
      gr[sl] = new TGraphErrors(c[sl].size(), &c[sl].at(0), &s[sl].at(0), &dc[sl].at(0), &ds[sl].at(0));
      gr[sl]->SetMarkerColor(2);
      gr[sl]->SetMarkerSize(1.0);
      gr[sl]->SetTitle(Form("Layer_%d;IWire;#LT t_{mea}-t_{fit} #GT [ns]", sl));
      gr[sl]->SetMaximum(10);
      gr[sl]->SetMinimum(-10);
      gr[sl]->SetName(Form("lay%d", sl));
      gr[sl]->Write();
    }
    fout->Close();
  }
  B2INFO("Write constants");
  Write();
  return true;
}
void Belle2::CDC::T0Correction::Write()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  ofstream ofs(m_outputT0FileName.c_str());
  DBImportObjPtr<CDCTimeZeros> tz;
  tz.construct();

  double T0;
  TH1F* T0B[300];
  for (int ib = 0; ib < 300; ++ib) {
    T0B[ib] = new TH1F(Form("T0B%d", ib), Form("boardID_%d", ib), 9000, 0, 9000);
  }
  //for calculate T0 mean of each board
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      WireID wireid(ilay, iwire);
      T0 = cdcgeo.getT0(wireid);
      T0B[cdcgeo.getBoardID(wireid)]->Fill(T0);
    }
  }

  //correct T0 and write
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      WireID wireid(ilay, iwire);
      int bID = cdcgeo.getBoardID(wireid);
      if (abs(err_dt[ilay][iwire]) > 2 || abs(dt[ilay][iwire]) > 1.e3) {
        //      if (abs(err_dt[ilay][iwire]) > 2) {
        T0 = T0B[bID]->GetMean();
        dt[ilay][iwire] = dtb[bID];
      } else {
        T0 = cdcgeo.getT0(wireid);
      }
      ofs <<  ilay << "\t" << iwire << "\t" << T0 - dt[ilay][iwire] << std::endl;
      if (m_useDB) {
        tz->setT0(wireid, T0 - dt[ilay][iwire]);
      }
    }
  }
  ofs.close();
  if (m_useDB) {
    IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                           m_lastExperiment, m_lastRun);
    tz.import(iov);
    B2RESULT("T0 was calibrated and imported to database.");
  }

}
