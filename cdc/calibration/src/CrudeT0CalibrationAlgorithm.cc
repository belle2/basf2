/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/calibration/CrudeT0CalibrationAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <framework/gearbox/Const.h>
#include <TError.h>
#include <TROOT.h>
#include <TF1.h>
#include <TFile.h>
#include <TTree.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>
using namespace std;
using namespace Belle2;
using namespace CDC;

CrudeT0CalibrationAlgorithm::CrudeT0CalibrationAlgorithm(): CalibrationAlgorithm("CDCCrudeT0Collector")
{

  setDescription(
    " -------------------------- T0 Calibration Algorithm -------------------------\n"
  );
}

void CrudeT0CalibrationAlgorithm::createHisto(StoreObjPtr<EventMetaData>& evtPtr)

{

  B2INFO("CreateHisto");

  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  const auto exprun =  getRunList();
  B2INFO("Changed ExpRun to: " << exprun[0].first << " " << exprun[0].second);
  evtPtr->setExperiment(exprun[0].first);
  evtPtr->setRun(exprun[0].second);
  DBStore::Instance().update();
  B2INFO("create TDChist");

  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      m_hTDC[il][w] = new TH1D(Form("hLay%d_ch%d", il, w), "tdc", m_tdcMax - m_tdcMin, m_tdcMin, m_tdcMax);
    }
  }

  B2INFO("create TDChist(board)");
  for (int ib = 0; ib < 300; ++ib) {
    m_hTDCBoard[ib] = new TH1D(Form("hTDCBoard%d", ib), "",  m_tdcMax - m_tdcMin, m_tdcMin, m_tdcMax);
  }
  m_hT0All = new TH1D("hT0All", "", 8500, 0, 8500);

  unsigned short lay;
  unsigned short wire;
  unsigned short tdc;
  auto tree = getObjectPtr<TTree>("tree");
  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("wire", &wire);
  tree->SetBranchAddress("tdc", &tdc);

  const int nEntries = tree->GetEntries();
  B2INFO("fill hist");
  for (int i = 0; i < nEntries ; ++i) {
    tree->GetEntry(i);
    m_hTDC[lay][wire]->Fill(tdc);
    m_hTDCBoard[cdcgeo.getBoardID(WireID(lay, wire))]->Fill(tdc);
    m_hT0All->Fill(tdc);
  }
  B2INFO("end of filling hist");
}

CalibrationAlgorithm::EResult CrudeT0CalibrationAlgorithm::calibrate()
{
  B2INFO("Start calibration");

  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;

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

  createHisto(evtPtr);

  TH1D* hs = new TH1D("hs", "sigma", 100, 0, 20);
  std::vector<double> sb;
  std::vector<double> dsb;
  std::vector<double> t0b;
  std::vector<double> dt0b;
  std::vector<double> b;
  std::vector<double> db;

  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  TF1* f1 = new TF1("f1", "[0]+[1]*(exp([2]*(x-[3]))/(1+exp(-([4]-x)/[5])))", m_tdcMin, m_tdcMax);
  f1->SetParLimits(0, 0., 1000.);
  f1->SetLineColor(kRed);
  const double tdcBinWidth = cdcgeo.getTdcBinWidth();
  bool bflag[300] = {false};

  for (int ib = 1; ib < 300; ++ib) {
    if (m_hTDCBoard[ib]->GetEntries() < m_minEntries) {
      B2DEBUG(199, "Warning: this board low statistic: " << m_hTDCBoard[ib]->GetEntries());
      bflag[ib] = false;
      m_t0b[ib] = m_initT0;
      continue;
    }
    double p3 = m_hTDCBoard[ib]->GetXaxis()->GetBinCenter(m_hTDCBoard[ib]->GetMaximumBin());
    f1->SetParameters(0, m_hTDCBoard[ib]->GetMaximum(), -0.001, p3, m_initT0, 2.5);
    m_hTDCBoard[ib]->Fit("f1", "QM", "", m_initT0 - 60, m_initT0 + 60);

    if ((fabs(f1->GetParameter(4) - m_initT0) > 100)
        || (fabs(f1->GetParameter(5)) < 0.01)
        || (fabs(f1->GetParameter(5)) > 16)) {

      bflag[ib] = false;
      m_t0b[ib] = m_initT0;
      continue;
    }

    bflag[ib] = true;
    m_t0b[ib] = f1->GetParameter(4) * tdcBinWidth;

    sb.push_back(f1->GetParameter(5));
    dsb.push_back(f1->GetParError(5));
    t0b.push_back(f1->GetParameter(4));
    dt0b.push_back(f1->GetParError(4));
    b.push_back(ib);
    db.push_back(0);
  }

  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      B2DEBUG(99, "fitting for channel: " << il << " - " << w);
      B2DEBUG(99, "number of entries" << m_hTDC[il][w]->GetEntries());
      m_t0[il][w] = m_initT0 * tdcBinWidth;
      int bid = cdcgeo.getBoardID(WireID(il, w));
      if (m_hTDC[il][w]->GetEntries() < m_minEntries) {
        B2DEBUG(99, "Warning: low statistic channel: " << m_hTDC[il][w]->GetEntries());
        if (bflag[bid] != false) {
          m_t0[il][w] = m_t0b[bid];
          m_flag[il][w] = true;
        } else {
          m_flag[il][w] = false;
        }
      } else {
        double p3 = m_hTDC[il][w]->GetXaxis()->GetBinCenter(m_hTDC[il][w]->GetMaximumBin());
        f1->SetParameters(0, m_hTDC[il][w]->GetMaximum(), -0.001, p3, m_initT0, 2.5);
        m_hTDC[il][w]->Fit("f1", "QM", "", m_initT0 - 60, m_initT0 + 60);
        B2DEBUG(99, "prob of fit : " << f1->GetProb());
        if ((f1->GetProb() < 1E-150) || (fabs(f1->GetParameter(4) - m_initT0) > 100) || (f1->GetParameter(5) < 0.1)
            || (f1->GetParameter(5) > 20)) {
          if (bflag[bid] != 0) {
            m_t0[il][w] = m_t0b[bid];
            m_flag[il][w] = true;
          } else {
            m_flag[il][w] = false;
          }
        } else {
          m_t0[il][w] = f1->GetParameter(4) * tdcBinWidth;
          hs->Fill(f1->GetParameter(5));
          m_flag[il][w] = true;
        }
      }

      B2DEBUG(99, "P4 = " << m_t0[il][w]);
      if (m_cosmic && cdcgeo.wireBackwardPosition(il, w).Y() > 0) {
        m_t0[il][w] -= cdcgeo.senseWireR(il) / Const::speedOfLight;
      } else {
        m_t0[il][w] += cdcgeo.senseWireR(il) / Const::speedOfLight;
      }
      m_t0[il][w] += (m_zOffset - cdcgeo.wireBackwardPosition(il, w).Z()) / 27.25;
      m_t0[il][w] += 6.122;
      m_hT0All->Fill(m_t0[il][w]);
    }
  }

  B2INFO("Write constants");
  write(evtPtr);
  saveHisto();
  return c_OK;
}

void CrudeT0CalibrationAlgorithm::write(StoreObjPtr<EventMetaData>& evtPtr)
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  const auto exprun =  getRunList();
  B2INFO("Changed ExpRun to: " << exprun[0].first << " " << exprun[0].second);
  evtPtr->setExperiment(exprun[0].first);
  evtPtr->setRun(exprun[0].second);
  DBStore::Instance().update();

  CDCTimeZeros* tz = new CDCTimeZeros();
  for (int ilay = 0; ilay < 56; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      WireID wireid(ilay, iwire);
      tz->setT0(wireid, m_t0[ilay][iwire]);
    }
  }
  saveCalibration(tz, "CDCTimeZeros");
}

void CrudeT0CalibrationAlgorithm::saveHisto()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  TFile* fhist = new TFile("histCrudeT0.root", "recreate");
  fhist->cd();
  TDirectory* top = gDirectory;
  TDirectory* Direct[56];
  for (int il = 0; il < 56; ++il) {
    top->cd();
    Direct[il] = gDirectory->mkdir(Form("lay_%d", il));
    Direct[il]->cd();
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      if (m_flag[il][w] == 1) {
        m_hTDC[il][w]->Write();
      }
    }
  }
  top->cd();
  TDirectory* board = gDirectory->mkdir("board");
  board->cd();
  for (int ib = 0; ib < 300; ++ib) {
    if (m_hTDCBoard[ib]) {
      m_hTDCBoard[ib]->Write();
    }
  }
  top->cd();
  m_hT0All->Write();

  fhist->Close();
}
