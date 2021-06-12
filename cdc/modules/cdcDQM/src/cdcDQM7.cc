/* Nanae Taniguchi 2017.07.12 */
/* Nanae Taniguchi 2018.02.06 */
/* add occupancy plot 2019.03 */
/* add occupancy hist for PV 2019.04 */
/* add occupancy hist for PV, updated 2019.04.28 */
/* update shifter's plots */

#include "cdc/modules/cdcDQM/cdcDQM7.h"

#include <framework/core/HistoModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TDirectory.h>

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
TH1D* h_hit_cell; // hit vs cell id

TH1D* h_EoccAfterInjLER; /*<nhits after LER injection>*/
TH1D* h_EoccAfterInjHER; /*<nhits after HER injection>*/
TH1D* h_occAfterInjLER; /*<occupancy after LER injection>*/
TH1D* h_occAfterInjHER; /*<occupancy after HER injection>*/


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
    } else if (b < 14) {
      bmax = ndiv[1];
    } else if (b < 20) {
      bmax = ndiv[2];
    } else if (b < 26) {
      bmax = ndiv[3];
    } else if (b < 32) {
      bmax = ndiv[4];
    } else if (b < 38) {
      bmax = ndiv[5];
    } else if (b < 44) {
      bmax = ndiv[6];
    } else if (b < 50) {
      bmax = ndiv[7];
    } else {
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

  // 20191108
  h_hit_cell = new TH1D("h_hit_cell", "Hit of each cell", 14336, 0, 14335);
  h_hit_cell->SetFillColor(20);
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

  h_EoccAfterInjLER = new TH1D("EoccAfterInjLer", "Eocc after LER injection", 4000, 0, 20000);
  h_EoccAfterInjLER->SetMinimum(0);
  h_EoccAfterInjLER->SetFillColor(7);

  h_EoccAfterInjHER = new TH1D("EoccAfterInjHer", "Eocc after HER injection", 4000, 0, 20000);
  h_EoccAfterInjHER->SetMinimum(0);
  h_EoccAfterInjHER->SetFillColor(7);

  h_occAfterInjLER = new TH1D("occAfterInjLer", "occupancy after LER injection", 4000, 0, 20000);
  h_occAfterInjLER->SetMinimum(0);
  h_occAfterInjLER->SetFillColor(7);

  h_occAfterInjHER = new TH1D("occAfterInjHer", "occupancy after HER injection", 4000, 0, 20000);
  h_occAfterInjHER->SetMinimum(0);
  h_occAfterInjHER->SetFillColor(7);

  oldDir->cd();//

}

void cdcDQM7Module::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager
  // register dataobjects
  m_rawFTSW.isOptional(); /// better use isRequired(), but RawFTSW is not in sim
  m_CDCRawHits.isRequired();
  m_CDCHits.isRequired();
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
  h_hit_cell->Reset();

  h_EoccAfterInjLER->Reset();
  h_EoccAfterInjHER->Reset();
  h_occAfterInjLER->Reset();
  h_occAfterInjHER->Reset();

}

void cdcDQM7Module::event()
{
  int nent = m_CDCHits.getEntries();
  int ftdc = 0;

  // occ total
  double occ_total = nent / 14336.;
  h_occ->Fill(occ_total);

  // for layer dependent occupancy
  int whits_L[56] = {}; // wire hits
  double occ_L[56] = {}; // occupancy

  int ndiv[9] = {160, 160, 192, 224, 256, 288, 320, 352, 384};

  for (int i = 0; i < nent; i++) {
    CDCHit* cdchit = static_cast<CDCHit*>(m_CDCHits[i]);

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

    // add by J.H. Yin
    if (adcsum > 25) {
      int cid(0);
      if (sL == 0) {
        cid = iL * ndiv[sL] + wid;
      } else {
        for (int isl = 0; isl < sL; isl ++) {
          cid += 6 * ndiv[isl];
        }
        cid += 2 * ndiv[0] + iL * ndiv[sL] + wid;
      }
      h_hit_cell -> Fill(cid);
    }
  }// cdchit

  h_fast_tdc->Fill(ftdc);

  // each layer
  for (int b = 0; b < 56; b++) {
    int n_wire = 0;
    if (b < 8) {
      n_wire = ndiv[0];
    } else if (b < 14) {
      n_wire = ndiv[1];
    } else if (b < 20) {
      n_wire = ndiv[2];
    } else if (b < 26) {
      n_wire = ndiv[3];
    } else if (b < 32) {
      n_wire = ndiv[4];
    } else if (b < 38) {
      n_wire = ndiv[5];
    } else if (b < 44) {
      n_wire = ndiv[6];
    } else if (b < 50) {
      n_wire = ndiv[7];
    } else {
      n_wire = ndiv[8];
    }

    // cal. occupancy
    occ_L[b] = (double)whits_L[b] / n_wire;
    h_occ_L[b]->Fill(occ_L[b]);
  }


  //
  int r_nent = m_CDCRawHits.getEntries();

  // new
  for (int j = 0; j < r_nent; j++) {
    CDCRawHit* cdcrawhit = static_cast<CDCRawHit*>(m_CDCRawHits[j]);

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
    } else if (brd < 48) {
      h_tdc_sL[1]->Fill(v_tdc);
      h_adc_sL[1]->Fill(v_adc);
    } else if (brd < 72) {
      h_tdc_sL[2]->Fill(v_tdc);
      h_adc_sL[2]->Fill(v_adc);
    } else if (brd < 100) {
      h_tdc_sL[3]->Fill(v_tdc);
      h_adc_sL[3]->Fill(v_adc);
    } else if (brd < 132) {
      h_tdc_sL[4]->Fill(v_tdc);
      h_adc_sL[4]->Fill(v_adc);
    } else if (brd < 168) {
      h_tdc_sL[5]->Fill(v_tdc);
      h_adc_sL[5]->Fill(v_adc);
    } else if (brd < 208) {
      h_tdc_sL[6]->Fill(v_tdc);
      h_adc_sL[6]->Fill(v_adc);
    } else if (brd < 252) {
      h_tdc_sL[7]->Fill(v_tdc);
      h_adc_sL[7]->Fill(v_adc);
    } else {
      h_tdc_sL[8]->Fill(v_tdc);
      h_adc_sL[8]->Fill(v_adc);
    }


  }// cdcrawhits

  for (auto& it : m_rawFTSW) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));
    auto difference = it.GetTimeSinceLastInjection(0);
    if (difference != 0x7FFFFFFF) {
      //unsigned int nentries = m_digits.getEntries();
      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      if (it.GetIsHER(0)) {
        h_occAfterInjHER->Fill(diff2, occ_total);
        h_EoccAfterInjHER->Fill(diff2);
      } else {
        h_occAfterInjLER->Fill(diff2, occ_total);
        h_EoccAfterInjLER->Fill(diff2);
      }
    }
  }




}


void cdcDQM7Module::endRun()
{
  //
}


void cdcDQM7Module::terminate()
{
  //
}
