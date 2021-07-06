/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "cdc/modules/cdcInitialT0Determination/CDCInitialT0Determination.h"
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Const.h>
#include "TF1.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "vector"
#include "TROOT.h"

using namespace std;
using namespace Belle2;
using namespace CDC;
REG_MODULE(CDCInitialT0Determination);

CDCInitialT0DeterminationModule::CDCInitialT0DeterminationModule() : Module()
{
  setDescription("Module to determine crude t0");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("OutputFileName", m_outputFileName, "t0 file output file", std::string("t0.dat"));
  addParam("LowTDC", m_tdcMin, "lower boundary of tdc histogram", m_tdcMin);
  addParam("UpTDC", m_tdcMax, "Upper boundary of tdc histogram", m_tdcMax);
  addParam("InitialT0", m_initT0, "initial t0 for fitting", 3579.);
  addParam("Cosmic", m_cosmic, "true; tof negative for upper part of cdc", true);
  addParam("Zoffset", m_zOffset, "z offset", 0.);
  addParam("ADCCut", m_adcMin, "threshold of ADC", m_adcMin);
  addParam("StoreFittedHisto", m_storeFittedHisto, "Store fitted histogram for each channel or not", true);
  addParam("HistoFileName", m_histoFileName, "file contain TDC histo", std::string("TDC.root"));
  addParam("MinEntries", m_minEntries, "minimum entries per channel", m_minEntries);
}

CDCInitialT0DeterminationModule::~CDCInitialT0DeterminationModule()
{
}


void CDCInitialT0DeterminationModule::initialize()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      m_hTDC[il][w] = new TH1D(Form("hLay%d_ch%d", il, w), "tdc", m_tdcMax - m_tdcMin, m_tdcMin, m_tdcMax);
    }
  }
  for (int ib = 0; ib < 300; ++ib) {
    m_hTDCBoard[ib] = new TH1D(Form("hTDCBoard%d", ib), "",  m_tdcMax - m_tdcMin, m_tdcMin, m_tdcMax);
  }
  m_hT0All = new TH1D("hT0All", "", 8500, 0, 8500);
  m_CDCHits.isRequired();
}
void CDCInitialT0DeterminationModule::event()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  for (const auto& hit : m_CDCHits) {
    WireID wireid(hit.getID());
    unsigned short lay = wireid.getICLayer();
    unsigned short w = wireid.getIWire();
    if (hit.getADCCount() > m_adcMin) {
      m_hTDC[lay][w]->Fill(hit.getTDCCount());
      m_hTDCBoard[cdcgeo.getBoardID(WireID(lay, w))]->Fill(hit.getTDCCount());
    }
  }
}
void CDCInitialT0DeterminationModule::terminate()
{
  gROOT->SetBatch(1);
  std::vector<double> sb;
  std::vector<double> dsb;
  std::vector<double> t0b;
  std::vector<double> dt0b;
  std::vector<double> b;
  std::vector<double> db;
  TH1D* hs = new TH1D("hs", "sigma", 100, 0, 20);

  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  TF1* f1 = new TF1("f1", "[0]+[1]*(exp([2]*(x-[3]))/(1+exp(-([4]-x)/[5])))", m_tdcMin, m_tdcMax);
  f1->SetParLimits(0, 0., 1000.);
  f1->SetLineColor(kRed);
  double tdcBinWidth = cdcgeo.getTdcBinWidth();
  double bflag[300];
  for (int ib = 1; ib < 300; ++ib) {
    if (m_hTDCBoard[ib]->GetEntries() < m_minEntries) {
      B2DEBUG(199, "Warning: this board low statistic: " << m_hTDCBoard[ib]->GetEntries());
      bflag[ib] = 0;
      m_t0b[ib] = m_initT0;
      continue;
    }
    double p3 = m_hTDCBoard[ib]->GetXaxis()->GetBinCenter(m_hTDCBoard[ib]->GetMaximumBin());
    f1->SetParameters(0, m_hTDCBoard[ib]->GetMaximum(), -0.001, p3, m_initT0, 2.5);
    m_hTDCBoard[ib]->Fit("f1", "QM", "", m_initT0 - 60, m_initT0 + 60);

    if ((fabs(f1->GetParameter(4) - m_initT0) > 100)
        || (fabs(f1->GetParameter(5)) < 0.01)
        || (fabs(f1->GetParameter(5)) > 16)) {

      bflag[ib] = 0;
      m_t0b[ib] = m_initT0;
      continue;
    }

    bflag[ib] = 1;
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
        if (bflag[bid] != 0) {
          m_t0[il][w] = m_t0b[bid];
          m_flag[il][w] = true;
        } else {m_flag[il][w] = false;}
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
          } else {m_flag[il][w] = false;}
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
      //      m_hT0b[cdcgeo.getBoardID(WireID(il, w))]->Fill(m_t0[il][w]);
    }
  }

  //check t0, and add t0 for low static channel
  ofstream ofs(m_outputFileName.c_str());
  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      if (m_flag[il][w] != true) {
        m_t0[il][w] = m_hT0All->GetMean();
      }
      ofs << il << "\t" << w << "\t" << m_t0[il][w] << endl;
    }
  }
  ofs.close();
  if (m_storeFittedHisto) {
    TFile* fhist = new TFile(m_histoFileName.c_str(), "recreate");
    fhist->cd();
    TDirectory* top = gDirectory;
    TDirectory* Direct[56];
    for (int il = 0; il < 56; ++il) {
      top->cd();
      Direct[il] = gDirectory->mkdir(Form("lay_%d", il));
      Direct[il]->cd();
      for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
        if (m_flag[il][w] == true) {
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
    hs->Write();
    if (b.size() > 20) {
      TGraphErrors* gr = new TGraphErrors(b.size(), &b.at(0), &sb.at(0), &db.at(0), &dsb.at(0));
      gr->SetName("reso");
      gr->Write();
      TGraphErrors* grT0b = new TGraphErrors(b.size(), &b.at(0), &t0b.at(0), &db.at(0), &dt0b.at(0));
      grT0b->SetName("T0Board");
      grT0b->Write();
    }
    fhist->Close();
  }
}

