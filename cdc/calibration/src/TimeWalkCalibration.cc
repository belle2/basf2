#include <cdc/calibration/TimeWalkCalibration.h>
#include <cdc/calibration/SliceFit.h>
//#include <calibration/CalibrationAlgorithm.h>
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
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

TimeWalkCalibration::TimeWalkCalibration()
{
  //  setDescription("CDC Time Walk Calibration");
}
void TimeWalkCalibration::CreateHisto()
{
  double x;
  double t_mea;
  double w;
  double t_fit;
  double ndf;
  double Pval;
  unsigned short adc;
  //  int board;
  int IWire;
  int lay;

  //  auto& tree = getObject<TTree>("cdcCalib");
  TChain* tree = new TChain("tree");
  tree->Add(m_InputRootFileName.c_str());

  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("IWire", &IWire);
  tree->SetBranchAddress("x_u", &x);
  tree->SetBranchAddress("t", &t_mea);
  tree->SetBranchAddress("t_fit", &t_fit);
  tree->SetBranchAddress("weight", &w);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("adc", &adc);
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  double halfCSize[56];
  for (int i = 0; i < 56; ++i) {
    double R = cdcgeo.senseWireR(i);
    double nW = cdcgeo.nWiresInLayer(i);
    halfCSize[i] = M_PI * R / nW;
  }

  //Histogram of each board
  for (int i = 0; i < 300; ++i) {
    m_h2[i] = new TH2D(Form("board_%d", i), Form("board_%d", i), 20, 0., 500, 60, -30, 30);
  }

  //read data
  B2INFO("Number of entry: " << tree->GetEntries());
  for (int i = 0; i < tree->GetEntries(); ++i) {
    tree->GetEntry(i);
    double xmax = halfCSize[lay] - 0.1;
    if ((fabs(x) < m_xmin) || (fabs(x) > xmax)
        || (ndf < m_ndfmin)
        || (Pval < m_Pvalmin)) continue; /*select good region*/

    m_h2[cdcgeo.getBoardID(WireID(lay, IWire))]->Fill(adc, fabs(t_mea) - fabs(t_fit));
  }
  B2INFO("Finish make histogram for all channels");
}

bool TimeWalkCalibration::calibrate()
{
  gROOT->SetBatch(1);
  CreateHisto();

  TF1* func = new TF1("func", "[0]+[1]/sqrt(x)", 0, 500);
  func->SetParameters(-4, 28);
  for (int ib = 1; ib < 300; ++ib) {
    B2DEBUG(199, "Board ID:" << ib);
    m_h2[ib]->SetDirectory(0);
    if (m_h2[ib]->GetEntries() < 500) {m_tw[ib] = 0; continue;}
    m_h1[ib] = (TH1D*)SliceFit::doSliceFitY(m_h2[ib], 20)->Clone();
    if (!m_h1[ib]) {m_tw[ib] = 0; continue;}
    m_h1[ib]->SetDirectory(0);
    if (m_h1[ib]->GetEntries() < 5) {m_tw[ib] = 0; continue;}
    B2DEBUG(199, "Number of point" << m_h1[ib]->GetEntries());
    m_h1[ib]->Fit("func", "MQ", "", 20, 150);
    m_tw[ib] = func->GetParameter(1);
    B2DEBUG(199, "Prob of fitting:" << func->GetProb());
    B2DEBUG(199, "Fitting Param 0-1:" << func->GetParameter(0) << " - " << func->GetParameter(1));
  }

  //Write histogram to file
  if (m_storeHisto) {
    B2INFO("Storing histogram");
    TFile* fhist = new TFile("tw_histo.root", "recreate");
    TDirectory* old = gDirectory;
    TDirectory* h1D = old->mkdir("h1D");
    TDirectory* h2D = old->mkdir("h2D");
    h1D->cd();
    for (int ib = 1; ib < 300; ++ib) {
      if (!m_h1[ib]) continue;
      if (m_h1[ib]->GetEntries() < 5) continue;
      m_h1[ib]->SetMinimum(-5);
      m_h1[ib]->SetMaximum(5);
      m_h1[ib]->Write();
    }
    h2D->cd();
    for (int ib = 1; ib < 300; ++ib) {
      if (m_h2[ib]) {
        m_h1[ib]->SetDirectory(0);
        m_h2[ib]->Write();
      }
    }
    fhist->Close();
    B2INFO("Hitograms were stored");
  }

  Write();
  return true;
}
void TimeWalkCalibration::Write()
{
  //for database
  B2INFO("update time walk params");
  if (m_useDB) {
    DBObjPtr<CDCTimeWalks> oldDB;
    DBImportObjPtr<CDCTimeWalks> tw;
    tw.construct();
    for (int ib = 0; ib < 300; ++ib) {
      //temp      double tw_old = oldDB->getTimeWalkParam(ib);
      //temp      tw->setTimeWalkParam(ib, tw_old + m_tw[ib]);
    }
    IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                           m_lastExperiment, m_lastRun);
    tw.import(iov);
    B2RESULT("Time-walk coeff. table imported to database.");
  } else {
    //For text mode
    std::string fileName1 = "/cdc/data/" + m_InputTWFileName;
    std::string fileName = FileSystem::findFile(fileName1);
    ifstream ifs;
    if (fileName == "") {
      fileName = FileSystem::findFile(m_InputTWFileName);
    }
    if (fileName == "") {
      B2FATAL("CDCGeometryPar: " << fileName1 << " not exist!");
    } else {
      B2INFO("Time Walk Calibration: open " << fileName);
      ifs.open(fileName.c_str());
      if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName1 << " !");
    }
    ofstream ofs(m_OutputTWFileName.c_str());
    unsigned short iBoard(0);
    int nRead(0);
    double coeff = 0.;
    while (true) {
      ifs >> iBoard >> coeff;
      if (ifs.eof()) break;
      ofs << iBoard << "  " << coeff + m_tw[iBoard] << endl;
      B2DEBUG(199, "Board: " << iBoard << " | tw_old" << coeff << " |dTW: " << m_tw[iBoard]);
      ++nRead;
    }
    if (nRead != 300) B2FATAL("#lines read-in (=" << nRead << ") is not equal #boards 300 !");
    ifs.close();
    ofs.close();
    B2RESULT("Time-walk coeff. table wrote to ." << m_OutputTWFileName.c_str());
  }
}
