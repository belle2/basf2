#include "cdc/modules/cdcInitialT0Determination/CDCInitialT0Determination.h"
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include "iostream"
#include "TF1.h"
#include "TDirectory.h"
#include "TFile.h"

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
}

CDCInitialT0DeterminationModule::~CDCInitialT0DeterminationModule()
{
}


void CDCInitialT0DeterminationModule::initialize()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      m_hTDC[il][w] = new TH1D(Form("h_Lay%d_ch%d", il, w), "tdc", m_tdcMax - m_tdcMin, m_tdcMin, m_tdcMax);
    }
  }
  for (int ib = 0; ib < 300; ++ib) {
    m_hT0b[ib] = new TH1D(Form("hT0b%d", ib), "", 8500, 0, 8500);
  }
  m_hT0All = new TH1D("hT0All", "", 8500, 0, 8500);

}
void CDCInitialT0DeterminationModule::event()
{
  StoreArray<CDCHit> cdcHits;
  for (const auto& hit : cdcHits) {
    WireID wireid(hit.getID());
    unsigned short lay = wireid.getICLayer();
    unsigned short w = wireid.getIWire();
    if (hit.getADCCount() > m_adcMin) {
      m_hTDC[lay][w]->Fill(hit.getTDCCount());
    }
  }
}
void CDCInitialT0DeterminationModule::terminate()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  TF1* f1 = new TF1("f1", "[0]+[1]*(exp([2]*(x-[3]))/(1+exp(-([4]-x)/[5])))", m_tdcMin, m_tdcMax);
  f1->SetParLimits(0, 0., 1000.);
  f1->SetLineColor(kRed);
  double tdcBinWidth = cdcgeo.getTdcBinWidth();

  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      B2DEBUG(99, "fitting for channel: " << il << " - " << w);
      B2DEBUG(99, "number of entries" << m_hTDC[il][w]->GetEntries());
      if (m_hTDC[il][w]->GetEntries() < 30) {
        B2DEBUG(99, "Warning: low statistic channel: " << m_hTDC[il][w]->GetEntries());
        m_flag[il][w] = 0;
        continue;
      }
      double p3 = m_hTDC[il][w]->GetXaxis()->GetBinCenter(m_hTDC[il][w]->GetMaximumBin());
      f1->SetParameters(0, m_hTDC[il][w]->GetMaximum(), -0.001, p3, m_initT0, 2.5);
      m_hTDC[il][w]->Fit("f1", "QM", "", m_initT0 - 60, m_initT0 + 40);
      B2DEBUG(99, "prob of fit : " << f1->GetProb());
      if (f1->GetProb() < 1E-10) {
        m_flag[il][w] = 0;
        continue;
      }

      m_t0[il][w] = f1->GetParameter(4) * tdcBinWidth;
      B2DEBUG(99, "P4 = " << m_t0[il][w]);
      if (m_cosmic && cdcgeo.wireBackwardPosition(il, w).Y() > 0) {
        //        m_t0[il][w] -= cdcgeo.senseWireR(il) / 29.8;
        m_t0[il][w] -= cdcgeo.senseWireR(il) / Const::speedOfLight;
      } else {
        //        m_t0[il][w] += cdcgeo.senseWireR(il) / 29.8;
        m_t0[il][w] += cdcgeo.senseWireR(il) / Const::speedOfLight;
      }
      m_t0[il][w] += (m_zOffset - cdcgeo.wireBackwardPosition(il, w).Z()) / 27.25;
      m_t0[il][w] += 6.122;

      m_hT0b[cdcgeo.getBoardID(WireID(il, w))]->Fill(m_t0[il][w]);
      m_hT0All->Fill(m_t0[il][w]);
      m_flag[il][w] = 1;
    }
  }
  //check t0, and add t0 for low static channel
  ofstream ofs(m_outputFileName.c_str());
  for (int il = 0; il < 56; ++il) {
    for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
      if (m_flag[il][w] != 1) {
        B2DEBUG(99, "this channel bad fit or low statistic");
        int boardID = cdcgeo.getBoardID(WireID(il, w));
        if (m_hT0b[boardID]->GetEntries() > 3) { //if t0 of this board exists
          m_t0[il][w] = m_hT0b[boardID]->GetMean();
        } else {
          m_t0[il][w] = m_hT0All->GetMean();
        } //use mean of all t0
      }
      ofs << il << "\t" << w << "\t" << m_t0[il][w] << endl;
    }
  }
  ofs.close();
  if (m_storeFittedHisto) {
    TFile* fhist = new TFile(m_histoFileName.c_str(), "recreate");
    fhist->cd();
    for (int il = 0; il < 56; ++il) {
      for (unsigned short w = 0; w < cdcgeo.nWiresInLayer(il); ++w) {
        if (m_flag[il][w] == 1) {
          m_hTDC[il][w]->Write();
        }
      }
    }
    m_hT0All->Write();
    fhist->Close();
  }
}

