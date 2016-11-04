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

namespace Belle2 {
  class TRGCDC;
  class TRGCDCSegmentHit;
  class TRGCDCLink;

  class TRGCDCEventTime {
  public:

    TRGCDCEventTime(const TRGCDC&, bool makeRootFile);

    virtual ~TRGCDCEventTime();

  public :

    void initialize(void);
    void terminate(void);
    void doit(int ver, bool print);
    void hitcount(void);
    void hist(void);
    void oldVer(void);
    void printFirm(void);
    int getT0(void)const;


  private :
    const TRGCDC& _cdc;

    TFile* m_fileEvtTime;
    TTree* m_evtOutputTs;
    TTree* m_evtOut;
    TH1* h;

    int m_fastestT;
    int m_histT;
    int cnt[9][64];
    int ft[9][64][10];
    bool m_foundT0;
    int m_foundT;
    int m_whdiff;
    int m_minusET;
    int m_noET;
//    int m_allET;
//    int m_yesET;
    int threshold;

    int m_eventN;

    bool m_makeRootFile;
    int m_ver;

  };
} // namespace Belle2

#endif
