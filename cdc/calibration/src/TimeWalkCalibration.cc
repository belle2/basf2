#include <cdc/calibration/TimeWalkCalibration.h>
//#include <calibration/CalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

#include <TMinuit.h>
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
  int nEntries = tree->GetEntries();
  B2INFO("Number of entry: " << nEntries);
  //Histogram of each board
  for (int i = 0; i < 300; ++i) {
    m_h2[i] = new TH2D(Form("board_%d", i), Form("board_%d", i), 50, 0., 500, 60, -30, 30);
  }

  //read data
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    double xmax = halfCSize[lay] - 0.12;
    if ((fabs(x) < m_xmin) || (fabs(x) > xmax)
        || (ndf < m_ndfmin)
        || (Pval < m_Pvalmin)) continue; /*select good region*/

    m_h2[cdcgeo.getBoardID(WireID(lay, IWire))]->Fill(adc, fabs(t_mea) - fabs(t_fit));
  }
  B2INFO("Reading data and filling histogram are done");
}

bool TimeWalkCalibration::calibrate()
{
  gROOT->SetBatch(1);
  readTW();
  CreateHisto();
  TF1* fold;
  if (m_twParamMode_old == 0)
    fold = new TF1("fold", "[0]/sqrt(x)", 0, 500);
  else if (m_twParamMode_old == 1)
    fold = new TF1("fold", "[0]*exp(-1*[1]*x)", 0, 500);
  else
    B2FATAL("Mode " << m_twParamMode_new << " haven't implemented yet.");

  B2INFO("Old time walk formular: ");
  fold->Print();
  B2INFO("Time walk mode new: " << m_twParamMode_new << " with " << m_nTwParams_new << " parameters");
  //  double p3, p1;
  m_tw_new[0].resize(m_nTwParams_new, 0.0); //for board 0, no available
  for (int ib = 1; ib < 300; ++ib) {
    m_flag[ib] = 1;
    B2DEBUG(199, "Board ID:" << ib);
    m_h2[ib]->SetDirectory(0);

    // ignore if histogram is low statistic
    if (m_h2[ib]->GetEntries() < 500) { m_flag[ib] = 0; continue;}
    // do slide fit
    m_h2[ib]->FitSlicesY(0, 1, -1, 10);
    TString name = m_h2[ib]->GetName();
    TString hm_name = name + "_1";
    m_h1[ib] = (TH1D*)gDirectory->Get(hm_name);
    if (!m_h1[ib]) {m_flag[ib] = 0; continue;}
    m_h1[ib]->SetDirectory(0);

    //if number of poist after slide fit quite low, ignore
    if (m_h1[ib]->GetEntries() < 5) {
      m_flag[ib] = 0;
      B2WARNING("Low statistic, number of points after slide fit: " << m_h1[ib]->GetEntries());
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
    }//fit for mode 0
    else if (m_twParamMode_new == 1) {
      fitToExponentialFunc(m_h1[ib]);//do fitting for mode 1
    } else {
      B2FATAL("Mode " << m_twParamMode_new << " is not available, please check");
    }

    // read fitted parameters
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
    TFile* fhist = new TFile("tw_histo.root", "recreate");
    TDirectory* old = gDirectory;
    TDirectory* h1D = old->mkdir("h1D");
    TDirectory* h2D = old->mkdir("h2D");
    h1D->cd();
    for (int ib = 1; ib < 300; ++ib) {
      if (!m_h1[ib] || m_flag[ib] != 1) continue;
      m_h1[ib]->SetDirectory(0);
      if (m_h1[ib]->GetEntries() < 5) continue;

      m_h1[ib]->SetMinimum(-5);
      m_h1[ib]->SetMaximum(15);
      m_h1[ib]->Write();
    }
    B2DEBUG(199, "Store 2D histogram");
    h2D->cd();
    for (int ib = 1; ib < 300; ++ib) {
      if (m_h2[ib]) {
        m_h2[ib]->SetDirectory(0);
        m_h2[ib]->Write();
      }
    }
    fhist->Close();
    B2INFO("All hitograms are stored");
  }
  Write();
  updateT0();
  return true;
}
void TimeWalkCalibration::Write()
{
  B2INFO("update time walk params");
  int nfailure(-1);//for no exist board; board 0;
  //  CDCTimeWalks* dbTw = new CDCTimeWalks();
  DBImportObjPtr<CDCTimeWalks> dbTw;
  dbTw.construct();

  for (int ib = 0; ib < 300; ++ib) {
    // If fitting fail and new mode is same as previous input mode. use old param
    if (m_flag[ib] != 1) {
      nfailure += 1;
      if (m_twParamMode_old == m_twParamMode_new) {
        dbTw->setTimeWalkParams(ib, m_tw_old[ib]);
      } else {
        //and even calibrated fail but mode is different from previous.
        //in this case, param is zero
        m_tw_new[ib].resize(m_nTwParams_new, 0.0);
        dbTw->setTimeWalkParams(ib, m_tw_new[ib]);
      }
    } else {
      //Use new param if board is successfuly calibrated
      dbTw->setTimeWalkParams(ib, m_tw_new[ib]);
    }
  }
  dbTw->setTwParamMode(m_twParamMode_new);
  dbTw->outputToFile(m_outputTWFileName);
  if (m_useDB) {
    IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                           m_lastExperiment, m_lastRun);
    dbTw.import(iov);
  }

  B2RESULT("Failure to calibrate time-walk for " << nfailure << " board");
  B2RESULT("Time-walk coeff. table wrote to " << m_outputTWFileName.c_str());
}
void TimeWalkCalibration::updateT0()
{
  B2INFO("Add constant term into T0 database");
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  ofstream ofs(m_outputT0FileName.c_str());
  DBImportObjPtr<CDCTimeZeros> tz;// = new CDCTimeZeros();
  tz.construct();
  double T0;
  //correct T0 and write
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      WireID wireid(ilay, iwire);
      int bID = cdcgeo.getBoardID(wireid);
      T0 = cdcgeo.getT0(wireid);
      if (m_flag[bID] == 1) {
        ofs <<  ilay << "\t" << iwire << "\t" << T0 - m_constTerm[bID] << std::endl;
        if (m_useDB)
          tz->setT0(wireid, T0 - m_constTerm[bID]);
      } else {
        ofs <<  ilay << "\t" << iwire << "\t" << T0  << std::endl;
        if (m_useDB)
          tz->setT0(wireid, T0 - m_constTerm[bID]);
      }
    }
  }
  ofs.close();
  if (m_useDB) {
    IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                           m_lastExperiment, m_lastRun);
    tz.import(iov);
  }
  B2RESULT("update constant term of tw correction to T0 constant.");
}
void TimeWalkCalibration::fitToExponentialFunc(TH1D* h1)
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
void TimeWalkCalibration::readTW()
{
  if (m_useDB) {
    DBObjPtr<CDCTimeWalks> oldDB;
    m_twParamMode_old = oldDB->getTwParamMode();
    for (int ib = 0; ib < 300; ++ib) {
      m_tw_old[ib] = oldDB->getTimeWalkParams(ib);
    }
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

    unsigned short iBoard(0);
    int nRead(0);
    double dumy(0.);// dumy2(0.), dumy3(0.);
    ifs >> m_twParamMode_old >> m_nTwParams_old;
    while (ifs >> iBoard) {
      m_tw_old[iBoard].resize(m_nTwParams_old);
      for (int i = 0; i < m_nTwParams_old; ++i) {
        ifs >> dumy;
        m_tw_old[iBoard][i] = dumy;
      }
      ++nRead;
    }
    if (nRead - 1 != 299) B2FATAL("#lines read-in (=" << nRead << ") is not equal #boards 299 !");
    ifs.close();
    B2INFO("TW from database: mode = " << m_twParamMode_old << "  with " << m_nTwParams_old << " parameters");
  }
}
