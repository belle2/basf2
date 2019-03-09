//+
// File : RawFTSWFormat_latest.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 7 - Mar - 2016
//-

#ifndef RAWFTSWFORMAT_LATEST_H
#define RAWFTSWFORMAT_LATEST_H
#include <rawdata/dataobjects/RawFTSWFormat.h>

namespace Belle2 {

  /**
   * The Raw FTSW class ver.1 .
   *
   * This class stores the RAW data containing FTSW data(event #, trg time ).
   */
  class RawFTSWFormat_latest : public RawFTSWFormat {
  public:

    //! Default constructor
    RawFTSWFormat_latest();
    //! Constructor using existing pointer to raw data buffer
    //RawFTSWFormat_latest(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSWFormat_latest();

    /*     //! Get # of words in this buffer */
    /*     int GetNwords(int n); */

    //! Get # of words of header
    int GetNwordsHeader(int n) override;

    //! Get Node # ( should be "TTD " )
    unsigned int GetFTSWNodeID(int n) override;

    //! Get event #
    unsigned int GetEveNo(int n) override;

    //! Get a word containing ctime and trigger type info
    unsigned int GetTTCtimeTRGType(int n) override;

    //! get unixtime of the trigger
    unsigned int GetTTUtime(int n) override;

    //! Get ctime of the trigger
    int GetTTCtime(int n) override;

    //! Get trgtype
    int GetTRGType(int n) override;

    //! Get timeval from ctime and utime
    void GetTTTimeVal(int n, struct timeval* tv) override;

    //! Get timespec from ctime and utime
    void GetTTTimeSpec(int n, struct timespec* ts) override;

    //! Get time in ns since epoch from ctime and utime
    unsigned long long int GetTTTimeNs(int n) override;

    //! Get magic number for data corruption check
    unsigned int GetMagicTrailer(int n) override;



    //! check the data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) override;

    //! Exp# (10bit) run# (14bit) restart # (8bit)
    unsigned int GetExpRunSubrun(int n) override;

    //! Get run #
    int GetRunNo(int n) override;


    //! Get subrun #
    int GetSubRunNo(int n) override;

    //! get a word cotaining run # and subrun #
    int GetRunNoSubRunNo(int n) override;

    //! Get Exp #
    int GetExpNo(int n) override;

    //! HER injection = 1 or LER injection = 0
    int GetIsHER(int n) override;

    //! Get time since the last injection
    unsigned int GetTimeSinceLastInjection(int n) override;

    //! Get time since the previous trigger
    unsigned int GetTimeSincePrevTrigger(int n) override;

    //! Get a bunch number
    unsigned int GetBunchNumber(int n) override;

    //! Get a frame count
    unsigned int GetFrameCount(int n) override;

    //!header format
    enum {
      POS_NWORDS = 0, // same as v.1( late-DESY ver.)
      POS_HDR_NWORDS = 1, // same as v.1
      POS_NUMEVE_NUMNODES = 2, // same as v.1
      POS_EXP_RUN_NO = 3, // same as v.1
      POS_EVE_NO_1 = 4, // same as v.1
      POS_NA_1 = 5, // same as v.1
      POS_NODE_ID = 6, // same as v.1
      POS_NA_2 = 7 // same as v.1
    };

    //!data format
    enum {
      POS_TT_CTIME_TRGTYPE = 8, // same as v.1
      POS_TT_UTIME = 9, // same as v.1
      POS_EVE_NO_2 = 10,
      POS_FRAME_COUNT = 11,
      POS_TIME_SINCE_PREV_TRG = 12,
      POS_INJECTION_INFO = 13,
      POS_BUNCH_NUM = 14
    };

    //!trailer format
    enum {
      POS_RSVD_1 = 20, // same as v.1 but position is shifted
      POS_MAGIC_1 = 21 // same as v.1 but position is shifted
    };

    enum {
      INJ_HER_LER_MASK = 0x80000000,
      INJ_HER_LER_SHIFT = 31,
      EXP_MASK = 0xFFC00000,
      EXP_SHIFT = 22,
      RUNNO_MASK = 0x003FFF00,
      RUNNO_SHIFT = 8,
      SUBRUNNO_MASK = 0x000000FF,
      TTCTIME_MASK = 0x7FFFFFF0,
      TTCTIME_SHIFT = 4,
      TRGTYPE_MASK = 0xF
    };

    enum {
      SIZE_FTSW_PACKET = 22
    };
    //#endif

    enum {
      FTSW_MAGIC_TRAILER = 0x7FFF0000
    };

  protected :

  };

  inline unsigned int RawFTSWFormat_latest::GetExpRunSubrun(int n)
  {
    return (unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]);

  }


  inline int RawFTSWFormat_latest::GetRunNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawFTSWFormat_latest::GetSubRunNo(int n)
  {
    return (m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline int RawFTSWFormat_latest::GetRunNoSubRunNo(int n)
  {
    return ((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) &
            (RUNNO_MASK | SUBRUNNO_MASK));
  }

  inline int RawFTSWFormat_latest::GetExpNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

}

#endif
