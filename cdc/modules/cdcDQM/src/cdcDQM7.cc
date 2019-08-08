/* Nanae Taniguchi 2017.07.12 */
/* Nanae Taniguchi 2018.02.06 */
/* add occupancy plot 2019.03 */
/* add occupancy hist for PV 2019.04 */
/* add occupancy hist for PV, updated 2019.04.28 */
/* update shifter's plots */

#include "cdc/modules/cdcDQM/cdcDQM7.h"
// add
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
//
#include <framework/core/HistoModule.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>

#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"
#include <stdio.h>

#include <TDirectory.h>
#include <TStyle.h>

#include <TProfile.h>//

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

TH1D* h_tdc_sL[9] = {0}; // tdc each super layer
TH1D* h_adc_sL[9] = {0}; // adc each super layer

TH1D* h_fast_tdc; // fastest TDC in each event
TH1D* h_board_out_tdc; // out of range TDC
TH2D* bmap_2; // board(copper-finess) status map 2D

// add 20190205
TH1D* h_occ; // occupancy
TH1D* h_occ_L[56]; // occupancy layer dependent

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

    h_occ_L[b] = new TH1D(Form("h_occ_L%d", b), "occ. each layer", 100, 0, 1); //
    h_occ_L[b]->SetFillColor(96);
    h_occ_L[b]->SetMinimum(0);
    h_occ_L[b]->SetStats(0);

  }

  for (int s = 0; s < 9; s++) {
    h_tdc_sL[s] = new TH1D(Form("tdc_sL%d", s), Form("tdc sLayer %d", s), 250, 4200, 5200);
    h_tdc_sL[s]->SetMinimum(0);
    h_tdc_sL[s]->SetFillColor(6);

    h_adc_sL[s] = new TH1D(Form("adc_sL%d", s), Form("adc sLayer %d", s), 100, 0, 500);
    h_adc_sL[s]->SetMinimum(0);
    h_adc_sL[s]->SetFillColor(8);
  }

  h_fast_tdc = new TH1D("fast_tdc", "fastest TDC", 50, 4800, 5000);
  h_fast_tdc->SetFillColor(6);

  h_board_out_tdc = new TH1D("h_board_out_tdc", "out of range TDC", 300, 0, 300);
  h_board_out_tdc->SetFillColor(95);

  // 20190205
  h_occ = new TH1D("occ", "occ. total", 100, 0, 1.);
  h_occ->SetFillColor(95);

  //
  bmap_2 = new TH2D("bmap_2", "", 75, 0, 75, 4, 0, 4);

  // LIVE
  h_tdc_sL[6]->SetOption("LIVE");
  h_tdc_sL[6]->SetOption("hist");

  h_board_out_tdc->SetOption("LIVE");
  h_board_out_tdc->SetOption("hist");

  // set
  h_fast_tdc->SetStats(1);
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
    h_occ_L[i]->Reset();
  }

  for (int j = 0; j < 9; j++) {
    h_tdc_sL[j]->Reset();
    h_adc_sL[j]->Reset();
  }

  h_fast_tdc->Reset();
  h_board_out_tdc->Reset();
  bmap_2->Reset();
  h_occ->Reset();

}

void cdcDQM7Module::event()
{

  StoreArray<CDCHit> cdcHits;
  int nent = cdcHits.getEntries();
  int ftdc = 0;

  // occ total
  h_occ->Fill(nent / 14336.);

  // for layer dependent occupancy
  int whits_L[56] = {}; // wire hits
  double occ_L[56] = {}; // occupancy

  for (int i = 0; i < nent; i++) {
    CDCHit* cdchit = static_cast<CDCHit*>(cdcHits[i]);

    int sL = cdchit->getISuperLayer();
    int iL = cdchit->getILayer();
    int wid = cdchit->getIWire();
    int adcsum = cdchit->getADCCount();
    int vtdc = cdchit->getTDCCount();

    if (sL > 8) continue; // error
    if (iL > 8) continue; // error

    int num = sL * 6 + iL + 2;
    if (num > 55) continue; // error

    // wire hits
    if (sL == 0) {
      whits_L[iL]++;
    } else {
      whits_L[num]++;
    }

    if (adcsum > -1) {

      if (sL == 0) {
        h_nhits_L[iL]->Fill(wid);
      } else {
        h_nhits_L[num]->Fill(wid);
      }

      //
      if (vtdc > ftdc && adcsum > 20) {
        ftdc = vtdc;
      }// fastest

    }// adc
  }// cdchit

  h_fast_tdc->Fill(ftdc);

  // each layer
  int ndiv[9] = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  for (int b = 0; b < 56; b++) {
    int n_wire = 0;
    if (b < 8) {
      n_wire = ndiv[0];
    } else if (b >= 8 && b < 14) {
      n_wire = ndiv[1];
    } else if (b >= 14 && b < 20) {
      n_wire = ndiv[2];
    } else if (b >= 20 && b < 26) {
      n_wire = ndiv[3];
    } else if (b >= 26 && b < 32) {
      n_wire = ndiv[4];
    } else if (b >= 32 && b < 38) {
      n_wire = ndiv[5];
    } else if (b >= 38 && b < 44) {
      n_wire = ndiv[6];
    } else if (b >= 44 && b < 50) {
      n_wire = ndiv[7];
    } else if (b >= 50 && b < 56) {
      n_wire = ndiv[8];
    }

    // cal. occupancy
    occ_L[b] = (double)whits_L[b] / n_wire;
    h_occ_L[b]->Fill(occ_L[b]);
  }


  //
  StoreArray<CDCRawHit> cdcRawHits;
  int r_nent = cdcRawHits.getEntries();

  // new
  for (int j = 0; j < r_nent; j++) {
    CDCRawHit* cdcrawhit = static_cast<CDCRawHit*>(cdcRawHits[j]);

    int brd = cdcrawhit->getBoardId();
    int v_adc = cdcrawhit->getFADC();
    int v_tdc = cdcrawhit->getTDC();
    int n_tot = cdcrawhit->getTOT();
    int n_node = cdcrawhit->getNode();
    int n_fns = cdcrawhit->getFiness();

    bmap_2->Fill(n_node, n_fns);

    if (v_tdc > 5200 || v_tdc < 4200) {
      h_board_out_tdc->Fill(brd);
    }

    //    printf("%d, %d:\n", j, board);
    if (brd > 299) continue;
    if (n_tot < 4) continue;
    if (v_adc < 35) continue;

    // each sL
    if (brd < 28) {
      h_tdc_sL[0]->Fill(v_tdc);
      h_adc_sL[0]->Fill(v_adc);
    } else if (brd > 27 && brd < 48) {
      h_tdc_sL[1]->Fill(v_tdc);
      h_adc_sL[1]->Fill(v_adc);
    } else if (brd > 47 && brd < 72) {
      h_tdc_sL[2]->Fill(v_tdc);
      h_adc_sL[2]->Fill(v_adc);
    } else if (brd > 71 && brd < 100) {
      h_tdc_sL[3]->Fill(v_tdc);
      h_adc_sL[3]->Fill(v_adc);
    } else if (brd > 99 && brd < 132) {
      h_tdc_sL[4]->Fill(v_tdc);
      h_adc_sL[4]->Fill(v_adc);
    } else if (brd > 131 && brd < 168) {
      h_tdc_sL[5]->Fill(v_tdc);
      h_adc_sL[5]->Fill(v_adc);
    } else if (brd > 167 && brd < 208) {
      h_tdc_sL[6]->Fill(v_tdc);
      h_adc_sL[6]->Fill(v_adc);
    } else if (brd > 207 && brd < 252) {
      h_tdc_sL[7]->Fill(v_tdc);
      h_adc_sL[7]->Fill(v_adc);
    } else if (brd > 251) {
      h_tdc_sL[8]->Fill(v_tdc);
      h_adc_sL[8]->Fill(v_adc);
    }


  }// cdcrawhits


}


void cdcDQM7Module::endRun()
{
  //
}


void cdcDQM7Module::terminate()
{
  //
}
