/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#include <TFile.h>
#include <TTree.h>

#ifdef TRGCDC_SHORT_NAMES
#define TCEventTime TRGCDCEventTime
#endif

namespace Belle2 {
  class TRGCDC;

  /// A class of TRGCDC Event Time
  class TRGCDCEventTime {
  public:

    /// constructor of TRGCDCEventTime class
    TRGCDCEventTime(const TRGCDC&, bool makeRootFile);
    /// destructor of TRGCDCEventTime class
    virtual ~TRGCDCEventTime();
    /// copy constructor, deleted
    TRGCDCEventTime(TRGCDCEventTime&) = delete;
    /// assignment operator, deleted
    TRGCDCEventTime& operator=(TRGCDCEventTime&) = delete;

  public :

    /// initialize the class
    void initialize(void);
    /// terminate function
    void terminate(void);
    /// Calculate T0 based on ver
    void doit(int ver, bool print);
    /// hit count of TS
    void hitcount(void);
    /// making hostogram
    void hist(void);
    /// old version of calculation function
    void oldVer(void);
    /// Print info in firmware level
    void printFirm(void);
    /// Calculate T0
    int getT0(void)const;


  private :
    /// TRGCDC class
    const TRGCDC& _cdc;

    /// TFile pointer
    TFile* m_fileEvtTime;
    /// TTree pointer of the TFile
    TTree* m_evtOutputTs;
    /// TTree pointer of the TFile
    TTree* m_evtOut;
    /// TH1 pointer of the TFile
    TH1* h;

    /// The fastest time of TS
    int m_fastestT;
    /// calculated T0
    int m_histT;
    /// TS counter for each SL and clock
    int cnt[9][64];
    /// Fastest time array each SL and clock
    int ft[9][64][10];
    /// T0 is found or not
    bool m_foundT0;
    /// Found time of TS
    int m_foundT;
    /// Drift time of TS
    int m_whdiff;
    /// minus ET bin
    int m_minusET;
    /// no ET bin is looped
    int m_noET;
//    int m_allET;
//    int m_yesET;
    /// Threshold value
    int threshold;

    /// event number
    int m_eventN;

    /// make Root file or not
    bool m_makeRootFile;
    /// Version
    int m_ver;

  };
} // namespace Belle2

#endif
