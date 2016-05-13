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

namespace Belle2{
  TRGCDCEventTime::TRGCDCEventTime(const TRGCDC & TRGCDC)
    : _cdc(TRGCDC){
    
      m_fileEvtTime = new TFile("ET.root","RECREATE");
      
      m_evtOutputTs = new TTree("m_evtOutsTs", "EvtTime Outputs for TS");
      m_evtTsA = new TTree("m_evtTsA", "EvtTime Outputs for 20 TS");
      m_evtTsB = new TTree("m_evtTsB", "EvtTime Outputs for 10 TS");
      m_evtTsC = new TTree("m_evtTsC", "EvtTime Outputs for 5 TS");
      m_evtOutS = new TTree("m_evtOutS", "EvtTime Outputs for 20 TS");
      m_evtOutA = new TTree("m_evtOutA", "EvtTime Outputs for 20 TS");
      m_evtOutB = new TTree("m_evtOutB", "EvtTime Outputs for 10 TS");
      m_evtOutC = new TTree("m_evtOutC", "EvtTime Outputs for 5 TS");

      m_evtOutputTs->Branch("tsFastestTime", &m_fastestT, "m_fastestT/I");
      m_evtOutputTs->Branch("tsFoundTime", &m_foundT, "m_foundT/I");
      m_evtOutputTs->Branch("foundT_fastT_diff", &m_whdiff, "m_whdiff/I");

      m_evtTsA->Branch("tsFastestTime20", &m_fastestT, "m_fastestT/I");
      m_evtTsB->Branch("tsFastestTime10", &m_fastestT, "m_fastestT/I");
      m_evtTsC->Branch("tsFastestTime5", &m_fastestT, "m_fastestT/I");

      m_evtOutS->Branch("histTime", &m_histT, "m_histT/I");
      m_evtOutA->Branch("histTime20", &m_histT20, "m_histT20/I");
      m_evtOutA->Branch("diffT", &m_diffT, "m_diffT/I");
      m_evtOutB->Branch("histTime10", &m_histT10, "m_histT10/I");
      m_evtOutB->Branch("diffT20_10", &m_diffT20_10, "m_diffT20_10/I");
      m_evtOutC->Branch("histTime5", &m_histT5, "m_histT5/I");
      m_evtOutC->Branch("diffT20_5", &m_diffT20_5, "m_diffT20_5/I");
      m_evtOutC->Branch("diffT10_5", &m_diffT10_5, "m_diffT10_5/I");
    
    }
  TRGCDCEventTime::~TRGCDCEventTime(){
    delete m_evtOutputTs;
    delete m_evtTsA;
    delete m_evtTsB;
    delete m_evtTsC;
    delete m_evtOutS;
    delete m_evtOutA;
    delete m_evtOutB;
    delete m_evtOutC;
    delete m_fileEvtTime;
  }

  void TRGCDCEventTime::initialize(void){
    m_minusET = 0;
    m_noET = 0;
    m_noET20 = 0;
    m_noET10 = 0;
    m_noET5 = 0;
    m_over5 = 0;
    m_over10 = 0;
    m_allET = 0;
    m_yesET = 0;
    m_eventN = 0;
  }

  void TRGCDCEventTime::doit(void){
    hist();
    getT0();
  }

  void TRGCDCEventTime::hist(void){

    m_eventN +=1;
    m_eventT=65535;
    m_foundT0 = 0;
    m_falseTsHit = 0;
    int threshold = 3;
    int hc = 0;
    int hv = 0;
    int hc_lasts =  5;
    m_histT= 500;
    m_histT20= 500;
    m_histT10= 500;
    m_histT5= 500;
    const vector<const TCSHit*> tsh = _cdc.segmentHits();

    TH1* h = new TH1D("h", "h", 1000, -500, 499);
    TH1* h20 = new TH1D("h20", "h20", 1000, -500, 499);
    TH1* h10 = new TH1D("h10", "h10", 1000, -500, 499);
    TH1* h5 = new TH1D("h5", "h5", 1000, -500, 499);
    m_tsfN = tsh.size();
    int cnt[9][64]={};
    int tempEN=-24555;

    for (int iClk=0; iClk<64; iClk++){
      for (int iTS=0; iTS<(int)tsh.size(); iTS++){
        const TRGCDCSegment & ts = tsh[iTS]->segment();
        int fndC = ts.foundTime()/16;
        m_fastestT = ts.fastestTime();
        m_foundT = ts.foundTime();
        m_whdiff = ts.foundTime() - ts.fastestTime();
        if(m_whdiff <= 256){
          if (iClk == fndC+31){
            h-> Fill(m_fastestT);
            m_evtOutputTs->Fill();
            for (int iSL=0; iSL<9; iSL++){
              if((int)ts.layer().id()==iSL){
                cnt[iSL][iClk]+=1;
                if(cnt[iSL][iClk]<=20){
                  h20->Fill(m_fastestT);
                  m_evtTsA->Fill();
                  if(cnt[iSL][iClk]<=10){
                    h10->Fill(m_fastestT);
                    m_evtTsB->Fill();
                    if(cnt[iSL][iClk]<=5){
                      h5->Fill(m_fastestT);
                      m_evtTsC->Fill();
                    }
                  }
                }
              }
            }
          }
        }
      }
      
      int j,k=0;
      if(iClk<32) j=0;
      else j=(iClk-32)*16;
      if(iClk<63) k=iClk*16;
      else k=1000;

      //for (int i = j; i<k; i++){
      //  if(h->GetBinContent(i) > threshold){
      //    m_histT = i-500;
      //    m_evtOutS->Fill();
      //    if(tempEN != m_eventN) m_yesET +=1;
      //    tempEN = m_eventN;
      //    m_allET +=1;
      //    if (m_histT <0) m_minusET +=1;
      //    break;
      //  }
      //}


      //    if(hc < hc_lasts){
      //      if(hc == 0){
      //        hc += 1;
      //        hv = m_histT;
      //      }
      //      else if(hv == m_histT){
      //        hc += 1;
      //        if(hc == hc_lasts){
      //          m_histT = hv;
      //          m_evtOutS->Fill();
      //          if(tempEN != m_eventN) m_yesET +=1;
      //          tempEN = m_eventN;
      //          m_allET +=1;
      //          if (m_histT <0) m_minusET += 1;
      //        }
      //      }
      //      else if(hv != m_histT){
      //        hc = 1;
      //        hv = m_histT;
      //      }
      //    }
      //    else{
      //      m_histT = hv;
      //      m_evtOutS->Fill();
      //      if(tempEN != m_eventN) m_yesET +=1;
      //      tempEN = m_eventN;
      //      m_allET +=1;
      //      if (m_histT <0){
      //        m_minusET += 1;
      //      }
      //    }
      //    break;
      //  }     
      //}
    }


    for (int iSL=0; iSL<9; iSL++){
      for (int iClk=0; iClk<64; iClk++){
        if(cnt[iSL][iClk] > 5){
          m_over5 +=1;
          if(cnt[iSL][iClk] > 10){
            m_over10 +=1;
          }
        }
      }
    }

//    for (int i=0; i<h->GetNbinsX(); i++){
    for (int i=450; i<600; i++){
      if(h->GetBinContent(i) > threshold){
        m_foundT0 = 1;
        m_histT = i-500;
        m_evtOutS->Fill();
        if (m_histT <0){
          m_minusET += 1;
        }
        break;
      }
    }//end of loop over hist bins 
    if(m_histT == 500){
      m_noET +=1;
    }//if no bin has contents over threshold no evt time

    for (int i=0; i<h20->GetNbinsX(); i++){
      if(h20->GetBinContent(i) > threshold){
        m_histT20 = i-500;
        m_diffT = m_histT - m_histT20;
        m_evtOutA->Fill();
        break;
      }
    }
    if(m_histT20 == 500){
      m_noET20 +=1;
    }

    for (int i=0; i<h10->GetNbinsX(); i++){
      if(h10->GetBinContent(i) > threshold){
        m_histT10 = i-500;
        m_diffT20_10 = m_histT20 - m_histT10;
        m_evtOutB->Fill();
        break;
      }
    }
    if(m_histT10 == 500){
      m_noET10 +=1;
    }

    for (int i=0; i<h5->GetNbinsX(); i++){
      if(h5->GetBinContent(i) > threshold){
        m_histT5 = i-500;
        m_diffT20_5 = m_histT20 - m_histT5;
        m_diffT10_5 = m_histT10 - m_histT5;
        m_evtOutC->Fill();
        break;
      }
    }
    if(m_histT5 == 500){
      m_noET5 +=1;
    }

    delete h;
    delete h20;
    delete h10;
    delete h5;
    
  }//end of TRGCDCEventTime::hist


  double TRGCDCEventTime::getT0 (void)const{
    TRGDebug::enterStage("Event Time");

    int et = m_histT;
    if(m_foundT0 == 0) et = 0;
    TRGDebug::leaveStage("Event Time");
    return et;
  }

  void TRGCDCEventTime::terminate(void){
    m_fileEvtTime->Write();
    m_fileEvtTime->Close();
//    cout << "allET : " << m_allET << endl;
//    cout << "minusET : " << m_minusET << endl;
//    cout << "noET : " << m_noET << endl;
//    cout << "yesET : " << m_yesET << endl;
//    cout << "noET20 : " << m_noET20 << endl;
//    cout << "noET10 : " << m_noET10 << endl;
//    cout << "noET5 : " << m_noET5 << endl;
//    cout << "m_over10 : " << m_over10 << endl;
//    cout << "m_over5 : " << m_over5 << endl;
  }
}
