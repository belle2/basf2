/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWFTSWFORMAT_LATEST_H
#define RAWFTSWFORMAT_LATEST_H
#include <rawdata/dataobjects/RawFTSWFormat.h>

namespace Belle2 {

  /**
   * The Raw FTSW class 3 ( 2019.8.20 )
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
    /* cppcheck-suppress missingOverride */
    int GetNwordsHeader(int n) OVERRIDE_CPP17;

    //! Get Node # ( should be "TTD " )
    /* cppcheck-suppress missingOverride */
    unsigned int GetFTSWNodeID(int n) OVERRIDE_CPP17;

    //! Get event #
    /* cppcheck-suppress missingOverride */
    unsigned int GetEveNo(int n) OVERRIDE_CPP17;

    //! Get a word containing ctime and trigger type info
    /* cppcheck-suppress missingOverride */
    unsigned int GetTTCtimeTRGType(int n) OVERRIDE_CPP17;

    //! get unixtime of the trigger
    /* cppcheck-suppress missingOverride */
    unsigned int GetTTUtime(int n) OVERRIDE_CPP17;

    //! Get ctime of the trigger
    /* cppcheck-suppress missingOverride */
    int GetTTCtime(int n) OVERRIDE_CPP17;

    //! Get trgtype
    /* cppcheck-suppress missingOverride */
    int GetTRGType(int n) OVERRIDE_CPP17;

    //! Get timeval from ctime and utime
    /* cppcheck-suppress missingOverride */
    void GetTTTimeVal(int n, struct timeval* tv) OVERRIDE_CPP17;

    //! Get timespec from ctime and utime
    /* cppcheck-suppress missingOverride */
    void GetTTTimeSpec(int n, struct timespec* ts) OVERRIDE_CPP17;

    //! Get time in ns since epoch from ctime and utime
    /* cppcheck-suppress missingOverride */
    unsigned long long int GetTTTimeNs(int n) OVERRIDE_CPP17;

    //! Get magic number for data corruption check
    /* cppcheck-suppress missingOverride */
    unsigned int GetMagicTrailer(int n) OVERRIDE_CPP17;

    //! Get stored information of getimeofday in a PC
    /* cppcheck-suppress missingOverride */
    void GetPCTimeVal(int n, struct timeval* tv) OVERRIDE_CPP17;

    //! check the data contents
    /* cppcheck-suppress missingOverride */
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) OVERRIDE_CPP17;

    //! Exp# (10bit) run# (14bit) restart # (8bit)
    /* cppcheck-suppress missingOverride */
    unsigned int GetExpRunSubrun(int n) OVERRIDE_CPP17;

    //! Get run #
    /* cppcheck-suppress missingOverride */
    int GetRunNo(int n) OVERRIDE_CPP17;


    //! Get subrun #
    /* cppcheck-suppress missingOverride */
    int GetSubRunNo(int n) OVERRIDE_CPP17;

    //! get a word cotaining run # and subrun #
    /* cppcheck-suppress missingOverride */
    int GetRunNoSubRunNo(int n) OVERRIDE_CPP17;

    //! Get Exp #
    /* cppcheck-suppress missingOverride */
    int GetExpNo(int n) OVERRIDE_CPP17;

    //! HER injection = 1 or LER injection = 0
    /* cppcheck-suppress missingOverride */
    int GetIsHER(int n) OVERRIDE_CPP17;

    //! Get time since the last injection
    /* cppcheck-suppress missingOverride */
    unsigned int GetTimeSinceLastInjection(int n) OVERRIDE_CPP17;

    //! Get time since the previous trigger
    /* cppcheck-suppress missingOverride */
    unsigned int GetTimeSincePrevTrigger(int n) OVERRIDE_CPP17;

    //! Get a bunch number
    /* cppcheck-suppress missingOverride */
    unsigned int GetBunchNumber(int n) OVERRIDE_CPP17;

    //! Get a frame count
    /* cppcheck-suppress missingOverride */
    unsigned int GetFrameCount(int n) OVERRIDE_CPP17;

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
      POS_BUNCH_NUM = 14,
      POS_TVSEC_FROM_PC = 15,
      POS_TVUSEC_FROM_PC = 16
    };

    //!trailer format
    enum {
      POS_RSVD_1 = 20, // same as v.1 but position is shifted
      POS_MAGIC_1 = 21 // same as v.1 but position is shifted
    };

    enum {
      INJ_HER_LER_MASK = 0x80000000,
      INJ_HER_LER_SHIFT = 31,
      INJ_TIME_MASK = 0x7FFFFFFF,
      INJ_TIME_SHIFT = 0,
      INJ_BUNCH_MASK = 0x000007FF,
      INJ_BUNCH_SHIFT = 0,
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
