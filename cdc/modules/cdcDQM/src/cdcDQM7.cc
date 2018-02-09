/* Nanae Taniguchi 2017.07.12 */
/* Nanae Taniguchi 2018.02.06 */

#include "cdc/modules/cdcDQM/cdcDQM7.h"

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>

#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"
#include <stdio.h>

#include <TDirectory.h>
#include <TStyle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(cdcDQM7)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

cdcDQM7Module::cdcDQM7Module() : HistoModule()
{
  //Set module properties
  setDescription("CDC DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);
}

cdcDQM7Module::~cdcDQM7Module()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

TH1D* h_nhits_L[56] = {0}; // hit in each layer
TH1D* h_tdc_L[56] = {0}; // tdc each layer
TH1D* h_adc_L[56] = {0}; // adc each layer

TH1D* h_tdc_sL[9] = {0}; // tdc each super layer
TH1D* h_adc_sL[9] = {0}; // adc each super layer

TH1D* h_fast_tdc; // fastest TDC in each event
TH2D* bmap_2; // board status map 2D

void cdcDQM7Module::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  TDirectory* dirDAQ = oldDir->mkdir("CDC");

  dirDAQ->cd();

  int bmin = 0;
  int bmax = 0;
  int ndiv[9] = {160, 160, 192, 224, 256, 288, 320, 352, 384};

  for (int b = 0; b < 56; b++) {

    if (b < 8) {
      bmax = ndiv[0];
    } else if (b >= 8 && b < 14) {
      bmax = ndiv[1];
    } else if (b >= 14 && b < 20) {
      bmax = ndiv[2];
    } else if (b >= 20 && b < 26) {
      bmax = ndiv[3];
    } else if (b >= 26 && b < 32) {
      bmax = ndiv[4];
    } else if (b >= 32 && b < 38) {
      bmax = ndiv[5];
    } else if (b >= 38 && b < 44) {
      bmax = ndiv[6];
    } else if (b >= 44 && b < 50) {
      bmax = ndiv[7];
    } else if (b >= 50 && b < 56) {
      bmax = ndiv[8];
    }

    // nhits_L
    h_nhits_L[b] = new TH1D(Form("nhits_L%d", b), Form("nhits Layer %d", b), bmax - bmin, bmin, bmax);
    h_nhits_L[b]->SetMinimum(0);
    h_nhits_L[b]->SetFillColor(7);

    // tdc
    h_tdc_L[b] = new TH1D(Form("tdc_L%d", b), Form("tdc Layer %d", b), 250, 4200, 5200);
    h_tdc_L[b]->SetMinimum(0);
    h_tdc_L[b]->SetFillColor(6);

    // adc
    h_adc_L[b] = new TH1D(Form("adc_L%d", b), Form("adc Layer %d", b), 75, 0, 150);
    h_adc_L[b]->SetMinimum(0);
    h_adc_L[b]->SetFillColor(8);

  }

  for (int s = 0; s < 9; s++) {
    h_tdc_sL[s] = new TH1D(Form("tdc_sL%d", s), Form("tdc sLayer %d", s), 250, 4200, 5200);
    h_tdc_sL[s]->SetMinimum(0);
    h_tdc_sL[s]->SetFillColor(6);

    h_adc_sL[s] = new TH1D(Form("adc_sL%d", s), Form("adc sLayer %d", s), 75, 0, 150);
    h_adc_sL[s]->SetMinimum(0);
    h_adc_sL[s]->SetFillColor(8);
  }

  h_fast_tdc = new TH1D("fast_tdc", "fastest TDC", 50, 4800, 5000);
  h_fast_tdc->SetFillColor(6);

  //

  bmap_2 = new TH2D("bmap_2", "", 20, 0, 20, 15, 0, 15);

  // LIVE
  h_tdc_sL[0]->SetOption("LIVE"); // small
  h_tdc_sL[8]->SetOption("LIVE"); // outer most
  h_adc_sL[0]->SetOption("LIVE"); // small
  h_adc_sL[8]->SetOption("LIVE"); // outer most
  h_fast_tdc->SetOption("LIVE"); // fastest TDC
  h_fast_tdc->SetStats(1);
  bmap_2->SetOption("LIVE"); //
  bmap_2->SetOption("zcol"); //
  bmap_2->SetStats(0);

  oldDir->cd();//

}

void cdcDQM7Module::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager

}

void cdcDQM7Module::beginRun()
{
  for (int i = 0; i < 56; i++) {
    h_nhits_L[i]->Reset();
    h_tdc_L[i]->Reset();
    h_adc_L[i]->Reset();
  }

  for (int j = 0; j < 9; j++) {
    h_tdc_sL[j]->Reset();
    h_adc_sL[j]->Reset();
  }

  h_fast_tdc->Reset();
  bmap_2->Reset();
  //
}

void cdcDQM7Module::event()
{

  StoreArray<CDCHit> cdcHits;
  int nent = cdcHits.getEntries();

  int sL = 0;
  int iL = 0;
  int wid = 0;
  int num = 0;
  int adcsum = 0;
  int vtdc = 0;
  int ftdc = 0;

  if (nent < 50 || nent > 250) return; //

  for (int i = 0; i < nent; i++) {
    CDCHit* cdchit = static_cast<CDCHit*>(cdcHits[i]);

    sL = cdchit->getISuperLayer();
    iL = cdchit->getILayer();
    wid = cdchit->getIWire();
    adcsum = cdchit->getADCCount();
    vtdc = cdchit->getTDCCount();

    num = sL * 6 + iL + 2;

    if (adcsum > 0) {

      if (sL == 0) {
        h_nhits_L[iL]->Fill(wid);
        h_tdc_L[iL]->Fill(vtdc);
        h_adc_L[iL]->Fill(adcsum);
        h_tdc_sL[sL]->Fill(vtdc);
        h_adc_sL[sL]->Fill(adcsum);
      } else {
        h_nhits_L[num]->Fill(wid);
        h_tdc_L[num]->Fill(vtdc);
        h_adc_L[num]->Fill(adcsum);
        h_tdc_sL[sL]->Fill(vtdc);
        h_adc_sL[sL]->Fill(adcsum);
      }

      if (vtdc > ftdc) {
        ftdc = vtdc;
      }// fastest

    }//adc

  }// cdchit

  h_fast_tdc->Fill(ftdc);

  //
  StoreArray<CDCRawHit> cdcRawHits;
  int r_nent = cdcRawHits.getEntries();

  // new
  int board = 0;
  int x = 0;
  int y = 0;

  for (int j = 0; j < r_nent; j++) {
    CDCRawHit* cdcrawhit = static_cast<CDCRawHit*>(cdcRawHits[j]);

    board = cdcrawhit->getBoardId();

    x = board % 20;
    y = (board - (board % 20)) / 20;
    bmap_2->Fill(x, y);

  }// cdcrawhits

  int h_ent = bmap_2->GetEntries();
  double fac = 20.*0.5;
  bmap_2->SetMaximum(h_ent / (300 * fac));

}


void cdcDQM7Module::endRun()
{
  //
}


void cdcDQM7Module::terminate()
{
  //
}
