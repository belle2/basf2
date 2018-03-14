//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : EventTime.cc
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to get Event Time information
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/EventTime.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <bitset>
#include <sstream>
#include <vector>

#include "cdc/dataobjects/CDCHit.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"

#include "TH1.h"
#include "TH1D.h"

using namespace std;

namespace Belle2 {

  TRGCDCEventTime::TRGCDCEventTime(const TRGCDC& TRGCDC, bool makeRootFile)
    : _cdc(TRGCDC),
      m_makeRootFile(makeRootFile)
  {

    if (m_makeRootFile) m_fileEvtTime = new TFile("ETF.root", "RECREATE");

    m_evtOutputTs = new TTree("m_evtOutsTs", "EvtTime Outputs for TS");
    m_evtOut = new TTree("m_evtOut", "EvtTime Outputs for 20 TS");

    m_evtOutputTs->Branch("tsFastestTime", &m_fastestT, "m_fastestT/I");
    m_evtOutputTs->Branch("tsFoundTime", &m_foundT, "m_foundT/I");
    m_evtOutputTs->Branch("foundT_fastT_diff", &m_whdiff, "m_whdiff/I");

    m_evtOut->Branch("histTime", &m_histT, "m_histT/I");

    h = new TH1D("h", "h", 1000, -500, 499);

  }
  TRGCDCEventTime::~TRGCDCEventTime()
  {
    delete m_evtOutputTs;
    delete m_evtOut;
    if (m_makeRootFile) delete m_fileEvtTime;
    delete h;
  }

  void TRGCDCEventTime::initialize(void)
  {
    m_minusET = 0;
    m_noET = 0;
    m_eventN = 0;
  }

  void TRGCDCEventTime::doit(int ver, bool print)
  {
    TRGDebug::enterStage("Event Time");
    m_eventN += 1;
    m_ver = ver;
    if (ver == 1) {
      oldVer();
      getT0();
    } else if (ver == 0) {
      hitcount();
      hist();
      getT0();
    } else if (ver == 2) {
      m_histT = 0;
      m_foundT0 = 1;
      getT0();
    } else {
      cout << "verETF error" << endl;
      cout << "choose 0:hist, 1:old, 2:trueT0" << endl;
    }
    if (print) printFirm();
    TRGDebug::leaveStage("Event Time");
  }

  void TRGCDCEventTime::hitcount(void)
  {
    TRGDebug::enterStage("Event Time");

    memset(cnt, 0, sizeof cnt);
    memset(ft, 0, sizeof ft);

    const vector<const TCSHit*> tsh = _cdc.segmentHits();

    for (int iClk = 0; iClk < 64; iClk++) {
      for (int iTS = 0; iTS < (int)tsh.size(); iTS++) {
        const TRGCDCSegment& ts = tsh[iTS]->segment();
        int fndC = ts.foundTime() / 16;
        m_fastestT = ts.fastestTime();
        m_foundT = ts.foundTime();
        m_whdiff = ts.foundTime() - ts.fastestTime();
        if (m_whdiff <= 256) {
          if (iClk == fndC + 31) {
            for (int iSL = 0; iSL < 9; iSL++) {
              if ((int)ts.layer().id() == iSL) {
                cnt[iSL][iClk] += 1;
                if (cnt[iSL][iClk] <= 10) {
                  h->Fill(m_fastestT);
                  m_evtOutputTs->Fill();
                  ft[iSL][iClk][cnt[iSL][iClk] - 1] = ts.fastestTime();
                }
              }
            }
          }
        }
      }
    }
    TRGDebug::leaveStage("Event Time");
  }//end of TRGCEventTime::hitcount

  void TRGCDCEventTime::hist(void)
  {
    TRGDebug::enterStage("Event Time");

    m_foundT0 = 0;
    threshold = 3;
    m_histT = 500;

    for (int i = 450; i < 600; i++) {
      if (h->GetBinContent(i) > threshold) {
        m_foundT0 = 1;
        m_histT = i - 500;
        m_evtOut->Fill();
        if (m_histT < 0) {
          m_minusET += 1;
        }
        break;
      }
    }//end of loop over hist bins
    if (m_histT == 500) {
      m_noET += 1;
    }//if no bin has contents over threshold no evt time
    h->Reset();

    TRGDebug::leaveStage("Event Time");
  }//end of TRGCDCEventTime::hist

  void TRGCDCEventTime::oldVer(void)
  {
    TRGDebug::enterStage("Event Time");
    m_histT = 65535;
    for (unsigned i = 0; i < _cdc.nSegmentLayers(); i++) {
      const Belle2::TRGCDCLayer* l = _cdc.segmentLayer(i);
      const unsigned nWires = l->nCells();
      for (unsigned j = 0; j < nWires; j++) {
        const TCSegment& s = (TCSegment&) * (*l)[j];
        const vector<const TCWire*>& wires = s.wires();
        const TRGSignal& timing = s.signal();
        if (timing.active()) {
          for (unsigned k = 0; k < wires.size(); k++) {
            if (wires[k]->hit()) {
              int dt = wires[k]->signal()[0]->time();
              if (m_histT > dt) {
                m_histT = dt;
                m_foundT0 = 1;
                m_evtOut->Fill();
              }
            }
          }
        }
      }
    }
    TRGDebug::leaveStage("Event Time");
  }//end of TRTGCDCEventTime::oldVer

  void TRGCDCEventTime::printFirm(void)
  {
    TRGDebug::enterStage("Event Time");

    for (int iClk = 0; iClk < 64; iClk++) {
      for (int iSL = 0; iSL < 9; iSL++) {
        int hmbw = 160 + 32 * (iSL - 1); //set bit width of hitmap part
        if (iSL == 0) hmbw = 160;
        string h[10];
        string t[10];
        char file[30];
        sprintf(file, "SL%d.coe", iSL);
        string cnts = string(cnt[iSL][iClk], '1');

        string clk = bitset<9>(iClk + 64 * (m_eventN - 1)).to_string();
        string clk2 = bitset<5>(iClk).to_string();
        for (int i = 0; i < 10; i++) {
          h[i] = bitset<4>(ft[iSL][iClk][i]).to_string();
          if (cnt[iSL][iClk] > i) t[i] = clk2 + h[i];
          else t[i] = "00000" + h[i];
        }
        ofstream fout;
        fout.open(file, ios::app);
        fout << clk;
        fout << setfill('0') << setw(hmbw) << cnts;
        fout << t[9] << t[8] << t[7] << t[6] << t[5] << t[4] << t[3] << t[2] << t[1] << t[0] << endl;
        fout.close();
      }
    }
    TRGDebug::leaveStage("Event Time");
  }//end of TRGCDCEventTime::printFirm


  int TRGCDCEventTime::getT0(void)const
  {
    int et = m_histT;
    if (m_foundT0 == 0) et = 9999;
    return et;
  }

  void TRGCDCEventTime::terminate(void)
  {
    delete h;
    if (m_makeRootFile) {
      m_fileEvtTime->Write();
      m_fileEvtTime->Close();
    }
  }
}
