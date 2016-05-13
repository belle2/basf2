//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : EventTime.h
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to get Event Time information
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------
#ifndef TRGCDCEventTime_FLAG_
#define TRGCDCEventTime_FLAG_

#include <vector>
#include <string>
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Cell.h"
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

#ifdef TRGCDC_SHORT_NAMES
#define TCEventTime TRGCDCEventTime
#endif

namespace Belle2{
  class TRGCDC;
  class TRGCDCSegmentHit;
  class TRGCDCLink;

  class TRGCDCEventTime {
    public:

      TRGCDCEventTime(const TRGCDC&);

      virtual ~TRGCDCEventTime();
    
    public : 
      
      void initialize(void);
      void terminate(void);
      void doit(void);
      void hist(void);
      void nl(void);
      double getT0(void)const;

   
    private : 
      const TRGCDC & _cdc;

      TFile* m_fileEvtTime;
      TTree* m_evtOutputTs;
      TTree* m_evtTsA;
      TTree* m_evtTsB;
      TTree* m_evtTsC;
      TTree* m_evtOutputEvt;
      TTree* m_evtOutS;
      TTree* m_evtOutA;
      TTree* m_evtOutB;
      TTree* m_evtOutC;
 
      int m_fastestT;
      int m_fastestTA;
      int m_fastestTB;
      int m_fastestTC;
      int m_eventT;
      int m_histT;
      int m_histT20;
      int m_histT10;
      int m_histT5;
      int m_diffT;
      int m_diffT20_10;
      int m_diffT20_5;
      int m_diffT10_5;
      double m_ewT;
      int m_ewTi;
      int m_hkT;
      int m_diffT1;
      int m_diffT2;
      bool m_foundT0;
      int m_tsfN;
      int m_foundT;
      int m_whdiff;
      int m_falseTsHit;
      int m_minusET;
      int m_noET;
      int m_noET20;
      int m_noET10;
      int m_noET5;
      int m_over5;
      int m_over10;
      int m_allET;
      int m_yesET;

      int m_eventN;

  };
} // namespace Belle2

#endif
