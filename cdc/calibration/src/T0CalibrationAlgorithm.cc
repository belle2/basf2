/**************************************************************************
- * basf2 (Belle II Analysis Software Framework)                           *
- * Author: The Belle II Collaboration                                     *
- *                                                                        *
- * See git log for contributors and copyright holders.                    *
- * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
- **************************************************************************/
#include <cdc/calibration/T0CalibrationAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TError.h>
#include <TROOT.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TFile.h>
#include <TTree.h>
#include <TStopwatch.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

T0CalibrationAlgorithm::T0CalibrationAlgorithm(): CalibrationAlgorithm("CDCCalibrationCollector")
{

  setDescription(
    " -------------------------- T0 Calibration Algorithm -------------------------\n"
  );
}

void T0CalibrationAlgorithm::createHisto()
{

  B2INFO("Creating histograms");
  Float_t x;
  Float_t t_mea;
  Float_t t_fit;
  Float_t ndf;
  Float_t Pval;
  UShort_t IWire;
  UChar_t lay;

  auto tree = getObjectPtr<TTree>("tree");
  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("IWire", &IWire);
  tree->SetBranchAddress("x_u", &x);
  tree->SetBranchAddress("t", &t_mea);
  tree->SetBranchAddress("t_fit", &t_fit);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);


  /* Disable unused branch */
  std::vector<TString> list_vars = {"lay", "IWire", "x_u", "t", "t_fit", "Pval", "ndf"};
  tree->SetBranchStatus("*", 0);

  for (TString brname : list_vars) {
    tree->SetBranchStatus(brname, 1);
  }


  double halfCSize[56];

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

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
      m_h1[il][ic] = new TH1F(Form("hdT_L%d_W%d", il, ic), Form("L%d_cell%d", il, ic), 30, -15, 15);
    }
  }

  //for each board
  for (int ib = 0; ib < 300; ++ib) {
    m_hT0b[ib] = new TH1F(Form("hdT_b%d", ib), Form("boardID_%d", ib), 100, -20, 20);
  }

  //read data
  const Long64_t nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  TStopwatch timer;
  timer.Start();
  for (Long64_t i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    double xmax = halfCSize[lay] - 0.1;
    if ((fabs(x) < m_xmin) || (fabs(x) > xmax)
        || (ndf < m_ndfmin)
        || (Pval < m_Pvalmin)) continue; /*select good region*/
    //each channel
    m_hTotal->Fill(t_mea - t_fit);
    m_h1[lay][IWire]->Fill(t_mea - t_fit);
    //each board
    int boardID = cdcgeo.getBoardID(WireID(lay, IWire));
    m_hT0b[boardID]->Fill(t_mea - t_fit);
  }
  timer.Stop();
  B2INFO("Finish making histogram for all channels");
  B2INFO("Time to fill histograms: " << timer.RealTime() << "s");

}

CalibrationAlgorithm::EResult T0CalibrationAlgorithm::calibrate()
{
  B2INFO("Start calibration");

  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;

  // We are potentially using data from several runs at once during execution
  // (which may have different DBObject values). So in general you would need to
  // average them, or aply them to the correct collector data.

  // However since this is the geometry lets assume it is fixed for now.
  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);

  // CDCGeometryPar basically constructs a ton of objects and other DB objects.
  // Normally we'd call updateDBObjPtrs to set the values of the requested DB objects.
  // But in CDCGeometryPar the DB objects get used during the constructor so they must
  // be set before/during the constructor.

  // Since we are avoiding using the DataStore EventMetaData, we need to pass in
  // an EventMetaData object to be used when constructing the DB objects.

  B2INFO("Creating CDCGeometryPar object");
  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  auto hEvtT0 =   getObjectPtr<TH1F>("hEventT0");
  double dEventT0 = hEvtT0->GetMean();
  createHisto();
  TH1F* hm_All = new TH1F("hm_All", "mean of #DeltaT distribution for all chanels", 500, -10, 10);
  TH1F* hs_All = new TH1F("hs_All", "#sigma of #DeltaT distribution for all chanels", 100, 0, 10);
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  TF1* g1 = new TF1("g1", "gaus", -100, 100);
  g1->SetParLimits(1, -20, 20);
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
    // Set Delta_T0=0 again to make sure there is no strange case
    // in which T0 might be initialed with a strange value
    dtb[ib] = 0;
    err_dtb[ib] = 0;

    if (m_hT0b[ib]->Integral(1, m_hT0b[ib]->GetNbinsX()) < 50) {
      //set error to large number as a flag of bad value
      err_dtb[ib] = 50; continue;
    }
    mean = m_hT0b[ib]->GetMean();
    m_hT0b[ib]->SetDirectory(0);
    m_hT0b[ib]->Fit("g1", "Q", "", mean - 15, mean + 15);
    g1->GetParameters(par);
    b.push_back(ib);
    db.push_back(0.0);
    Sb.push_back(par[1]);
    dSb.push_back(g1->GetParError(1));
    dtb[ib] = par[1] + dEventT0;// add dEvtT0 here
    err_dtb[ib] = g1->GetParError(1);
  }
  B2INFO("Gaus fitting for each cell");
  for (int ilay = 0; ilay < 56; ++ilay) {
    const unsigned int nW = cdcgeo.nWiresInLayer(ilay);
    for (unsigned int iwire = 0; iwire < nW; ++iwire) {

      const int n = m_h1[ilay][iwire]->Integral(1, m_h1[ilay][iwire]->GetNbinsX()) ;
      B2DEBUG(21, "layer " << ilay << " wire " << iwire << " entries " << n);
      // Set Delta_T0=0 again to make sure there is no strange case
      // in which T0 might be initialed with a strange value
      dt[ilay][iwire] = 0;
      err_dt[ilay][iwire] = 0;

      if (n < 30) {
        //set error to large number as a flag of bad value
        err_dt[ilay][iwire] = 50; continue;
      }

      mean = m_h1[ilay][iwire]->GetMean();
      m_h1[ilay][iwire]->SetDirectory(0);
      m_h1[ilay][iwire]->Fit("g1", "Q", "", mean - 15, mean + 15);
      g1->GetParameters(par);
      c[ilay].push_back(iwire);
      dc[ilay].push_back(0.0);
      s[ilay].push_back(par[1]);
      ds[ilay].push_back(g1->GetParError(1));

      dt[ilay][iwire] = par[1] + dEventT0; // add dEvtT0 here;
      err_dt[ilay][iwire] = g1->GetParError(1);
      hm_All->Fill(par[1]);// mean of gauss fitting.
      hs_All->Fill(par[2]); // sigma of gauss fitting.
    }
  }

  if (m_storeHisto) {
    B2INFO("Storing histograms");
    auto hNDF =   getObjectPtr<TH1F>("hNDF");
    auto hPval =   getObjectPtr<TH1F>("hPval");
    TFile* fout = new TFile(m_histName.c_str(), "RECREATE");
    fout->cd();
    TGraphErrors* gr[56];
    TDirectory* top = gDirectory;

    //store NDF, P-val. EventT0 histogram for monitoring during calibration
    if (hNDF && hPval && hEvtT0) {
      hEvtT0->Write();
      hPval->Write();
      hNDF->Write();
    }
    m_hTotal->Write();
    hm_All->Write();
    hs_All->Write();
    TDirectory* subDir[56];
    for (int ilay = 0; ilay < 56; ++ilay) {
      subDir[ilay] = top ->mkdir(Form("lay_%d", ilay));
      subDir[ilay]->cd();
      const unsigned int nW = cdcgeo.nWiresInLayer(ilay);
      for (unsigned int iwire = 0; iwire < nW; ++iwire) {
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
  B2INFO("Writing constants...");
  write();

  B2INFO("Checking conversion conditons...");
  double n_below = hm_All->Integral(0, hm_All->GetXaxis()->FindBin(m_offsetMeanDt - 0.5));
  double n_upper = hm_All->Integral(hm_All->GetXaxis()->FindBin(m_offsetMeanDt + 0.5), hm_All->GetXaxis()->GetNbins() - 1);
  B2INFO("+ Number of channel which need still need to be calibrated are: " << n_below + n_upper);
  B2INFO("+ Median of Delta_T - offset:" << hm_All->GetMean() - m_offsetMeanDt << "(requirement: " << m_maxMeanDt << ")");
  B2INFO("+ RMS of Delta_T dist. :" << hm_All->GetRMS() << "  (Requirement: < " << m_maxRMSDt << ")");

  if (fabs(hm_All->GetMean() - m_offsetMeanDt) < m_maxMeanDt
      && fabs(hm_All->GetRMS()) < m_maxRMSDt
      && n_below + n_upper < m_maxBadChannel) {
    B2INFO("T0 Calibration Finished:");
    return c_OK;
  } else {
    B2INFO("Need more iteration ...");
    return c_Iterate;
  }
}

void T0CalibrationAlgorithm::write()
{
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  CDCTimeZeros* tz = new CDCTimeZeros();

  TH1F* hT0B[300];
  for (int ib = 0; ib < 300; ++ib) {
    hT0B[ib] = new TH1F(Form("hT0B%d", ib), Form("boardID_%d", ib), 9000, 0, 9000);
  }

  TH1F* hT0_all = new TH1F("hT0_all", "T0 distribution", 9000, 0, 9000);
  double T0;
  // get old T0 to calculate T0 mean of each board
  for (int ilay = 0; ilay < 56; ++ilay) {
    const unsigned int nW = cdcgeo.nWiresInLayer(ilay);
    for (unsigned int iwire = 0; iwire < nW; ++iwire) {
      WireID wireid(ilay, iwire);
      T0 = cdcgeo.getT0(wireid);
      hT0_all->Fill(T0);
      hT0B[cdcgeo.getBoardID(wireid)]->Fill(T0);
    }
  }

  //get Nominal T0
  double  T0_average = getMeanT0(hT0_all);
  if (fabs(T0_average - m_commonT0) > 50) {B2WARNING("Large difference between common T0 (" << m_commonT0 << ") and aveage value" << T0_average);}
  //  get average T0 for each board and also apply T0 correction for T0 of that board
  double T0B[300];
  for (int i = 0; i < 300; ++i) {T0B[i] = m_commonT0;}


  for (int ib = 1; ib < 300; ++ib) {
    T0B[ib] = getMeanT0(hT0B[ib]);
    if (fabs(T0B[ib] - T0_average) > 25) {
      B2WARNING("T0 of Board " << ib << " (= " << T0B[ib] << ") is too different with common T0: " << T0_average <<
                "\n It will be replaced by common T0");
      T0B[ib] = T0_average;
      continue;
    }
    //correct T0 board
    if (abs(err_dtb[ib]) < 2 && abs(dtb[ib]) < 20) {
      T0B[ib] -= dtb[ib];
    }
  }

  //correct T0 and write
  double dT;
  for (int ilay = 0; ilay < 56; ++ilay) {
    const unsigned int nW = cdcgeo.nWiresInLayer(ilay);
    for (unsigned int iwire = 0; iwire < nW; ++iwire) {
      WireID wireid(ilay, iwire);
      int bID = cdcgeo.getBoardID(wireid);

      //get old T0, replace with common T0 of board or average over all channel.
      T0 = cdcgeo.getT0(wireid);
      if (fabs(T0 - T0B[bID]) > 25 || fabs(T0 - T0_average) > 25) {
        B2WARNING("T0 of wireID L-W: " << ilay << "--" << iwire << " (= " << T0
                  << ") is too different with common T0 of board: " << bID << " = " << T0B[bID] <<
                  "\n It will be replaced by common T0 of the Board");
        T0 = T0B[bID];
      }

      //select DeltaT
      dT = 0;
      if (abs(err_dt[ilay][iwire]) < 2 && abs(dt[ilay][iwire]) < 20) {
        dT = dt[ilay][iwire];
      }

      //export
      tz->setT0(wireid, T0 - dT);
    }
  }

  if (m_textOutput == true) {
    tz->outputToFile(m_outputT0FileName);
  }
  saveCalibration(tz, "CDCTimeZeros");
}
double T0CalibrationAlgorithm::getMeanT0(TH1F* h1)
{
  double mean1 = h1->GetMean();
  h1->GetXaxis()->SetRangeUser(mean1 - 50, 8000);
  double mean2 = h1->GetMean();
  while (fabs(mean1 - mean2) > 0.5) {
    mean1 = mean2;
    h1->GetXaxis()->SetRangeUser(mean1 - 50, 8000);
    mean2 = h1->GetMean();
  }
  return mean2;

}
