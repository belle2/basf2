//+
// File : RawFTSWFormat_v1.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 7 - Mar - 2016
//-

#ifndef RAWFTSWFORMAT_V1_H
#define RAWFTSWFORMAT_V1_H
#include <rawdata/dataobjects/RawFTSWFormat.h>

namespace Belle2 {

  /**
   * The Raw FTSW class ver.1 .
   *
   * This class stores the RAW data containing FTSW data(event #, trg time ).
   */
  class RawFTSWFormat_v1 : public RawFTSWFormat {
  public:

    //! Default constructor
    RawFTSWFormat_v1();
    //! Constructor using existing pointer to raw data buffer
    //RawFTSWFormat_v1(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSWFormat_v1();

    /*     //! Get # of words in this buffer */
    /*     int GetNwords(int n); */

    //! Get # of words of header
    int GetNwordsHeader(int n)  override;

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

    //! DESY test only
    int Get15bitTLUTag(int n) override;

    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_NUMEVE_NUMNODES = 2,
      POS_EXP_RUN_NO = 3,
      POS_EVE_NO = 4,
      POS_NA_1 = 5,
      POS_NODE_ID = 6,
      POS_NA_2 = 7,
      POS_TT_CTIME_TRGTYPE = 8,
      POS_TT_UTIME = 9,
      POS_FTSW_3 = 10,
      POS_FTSW_4 = 11,
      POS_RSVD_1 = 12,
      POS_MAGIC_1 = 13
    };

    enum {
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
      SIZE_FTSW_PACKET = 14
    };
    //#endif

    enum {
      FTSW_MAGIC_TRAILER = 0x7FFF0000
    };

  protected :

  };

  inline unsigned int RawFTSWFormat_v1::GetExpRunSubrun(int n)
  {
    return (unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]);

  }


  inline int RawFTSWFormat_v1::GetRunNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawFTSWFormat_v1::GetSubRunNo(int n)
  {
    return (m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline int RawFTSWFormat_v1::GetRunNoSubRunNo(int n)
  {
    return ((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) &
            (RUNNO_MASK | SUBRUNNO_MASK));
  }

  inline int RawFTSWFormat_v1::GetExpNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

}

#endif
