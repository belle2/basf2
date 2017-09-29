#include <cdc/calibration/T0Correction4CAF.h>
#include <calibration/CalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

#include <TError.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include "iostream"
#include "string"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>
using namespace std;
using namespace Belle2;
using namespace CDC;

T0Correction4CAF::T0Correction4CAF(): CalibrationAlgorithm("CDCT0CalibrationCollector")
{

  setDescription(" -------------------------- T0 Calibration Algoritm -------------------------\n"

                 " ------------------------------------------------------------------------------\n"
                );
}


CalibrationAlgorithm::EResult T0Correction4CAF::calibrate()
{
  B2INFO("Start calibration");

  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;


  //  CreateHisto();
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  TF1* g1 = new TF1("g1", "gaus", -100, 100);
  vector<double> b, db, Sb, dSb;
  vector<double> c[56];
  vector<double> dc[56];
  vector<double> s[56];
  vector<double> ds[56];

  B2INFO("Gaus fitting for whole channel");
  double par[3];
  m_hTotal = getObject<TH1F>("hTotal");
  m_hTotal.SetDirectory(0);
  B2DEBUG(99, "number of entries:" << m_hTotal.GetEntries());
  const double mean = m_hTotal.GetMean();
  m_hTotal.Fit("g1", "Q", "", mean - 15, mean + 15);
  g1->GetParameters(par);

  B2INFO("Gaus fitting for each board");
  for (int ib = 1; ib < 300; ++ib) {
    m_hT0b[ib] = getObject<TH1F>(Form("hT0b%d", ib));
    if (m_hT0b[ib].GetEntries() < 20) continue;
    const double meanb = m_hT0b[ib].GetMean();
    m_hT0b[ib].SetDirectory(0);
    m_hT0b[ib].Fit("g1", "Q", "", meanb - 15, meanb + 15);
    g1->GetParameters(par);
    b.push_back(ib);
    db.push_back(0.0);
    Sb.push_back(par[1]);
    dSb.push_back(g1->GetParError(1));
    dtb[ib] = par[1];
    err_dtb[ib] = g1->GetParError(1);
  }
  B2INFO("Gaus fitting for each cell");
  TH1D* hs = new TH1D("hs", "Sigma of delta T0 of all channel", 100, -15, 15);
  TH1D* hm = new TH1D("hm", "Sigma of delta T0 of all channel", 100, 0, 15);
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      m_h1[ilay][iwire] = getObject<TH1F>(Form("hdT_lay%d_cell%d", ilay, iwire));
      const int n = m_h1[ilay][iwire].GetEntries();
      B2DEBUG(99, "layer " << ilay << " wire " << iwire << " entries " << n);
      if (n < 10) continue;
      const double meanc = m_h1[ilay][iwire].GetMean();
      m_h1[ilay][iwire].SetDirectory(0);
      m_h1[ilay][iwire].Fit("g1", "Q", "", meanc - 15, meanc + 15);
      g1->GetParameters(par);
      c[ilay].push_back(iwire);
      dc[ilay].push_back(0.0);
      s[ilay].push_back(par[1]);
      ds[ilay].push_back(g1->GetParError(1));

      dt[ilay][iwire] = par[1];
      err_dt[ilay][iwire] = g1->GetParError(1);

      hm->Fill(par[1]);
      hs->Fill(par[2]);
    }
  }


  if (m_storeHisto) {
    B2INFO("Store histo");
    TFile* fout = new TFile("Correct_T0.root", "RECREATE");
    fout->cd();
    TGraphErrors* gr[56];
    TDirectory* top = gDirectory;
    m_hTotal.Write();
    hm->Write();
    hs->Write();
    TDirectory* corrT0 = top->mkdir("DeltaT0");
    corrT0->cd();

    TGraphErrors* grb = new TGraphErrors(b.size(), &b.at(0), &Sb.at(0), &db.at(0), &dSb.at(0));
    grb->SetMarkerColor(2);
    grb->SetMarkerSize(1.0);
    grb->SetTitle("#DeltaT0;BoardID;#DeltaT0[ns]");
    grb->SetMaximum(10);
    grb->SetMinimum(-10);
    grb->SetName("Board");
    grb->Write();

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
    if (m_debug) {
      TDirectory* Direct[56];
      for (int il = 0; il < 56; ++il) {
        top->cd();
        Direct[il] = gDirectory->mkdir(Form("lay_%d", il));
        Direct[il]->cd();
        for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
          m_h1[il][w].Write();
        }
      }
    }
    fout->Close();
  }
  B2INFO("Write constants");
  Write();
  //return true;
  if (fabs(hm->GetMean()) < 0.2 && hm->GetRMS() < 0.5) {
    return c_OK;
  } else {
    return c_Iterate;
  }

}
void T0Correction4CAF::Write()
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
