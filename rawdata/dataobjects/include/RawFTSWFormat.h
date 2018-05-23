//+
// File : RawFTSWFormat.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 7 - Mar - 2016
//-

#ifndef RAWFTSWFORMAT_H
#define RAWFTSWFORMAT_H
#include <rawdata/dataobjects/RawDataBlockFormat.h>

namespace Belle2 {

  /**
   * The Raw FTSW class.
   *
   * This class stores the RAW data containing FTSW data(event #, trg time ).
   */
  class RawFTSWFormat : public RawDataBlockFormat {
  public:

    //! Default constructor
    RawFTSWFormat();
    //! Constructor using existing pointer to raw data buffer
    //RawFTSWFormat(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSWFormat();

    /*     //! Get # of words in this buffer */
    /*     int GetNwords(int n); */

    //! Get # of words of header
    int GetNwordsHeader(int n);

    //! Get Node # ( should be "TTD " )
    unsigned int GetFTSWNodeID(int n);

    //! Get event #
    unsigned int GetEveNo(int n);

    //! Get a word containing ctime and trigger type info
    unsigned int GetTTCtimeTRGType(int n);

    //! get unixtime of the trigger
    unsigned int GetTTUtime(int n);

    //! Get ctime of the trigger
    int GetTTCtime(int n);

    //! Get trgtype
    int GetTRGType(int n);

    //! Get timeval from ctime and utime
    void GetTTTimeVal(int n, struct timeval* tv);

    //! Get timespec from ctime and utime
    void GetTTTimeSpec(int n, struct timespec* ts);

    //! Get magic number for data corruption check
    unsigned int GetMagicTrailer(int n);



    //! check the data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no);

    //! Exp# (10bit) run# (14bit) restart # (8bit)
    unsigned int GetExpRunSubrun(int n);

    //! Get run #
    int GetRunNo(int n);


    //! Get subrun #
    int GetSubRunNo(int n);

    //! get a word cotaining run # and subrun #
    int GetRunNoSubRunNo(int n);

    //! Get Exp #
    int GetExpNo(int n);

    //! DESY test only
    int Get15bitTLUTag(int n);

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

  inline unsigned int RawFTSWFormat::GetExpRunSubrun(int n)
  {
    return (unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]);

  }


  inline int RawFTSWFormat::GetRunNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawFTSWFormat::GetSubRunNo(int n)
  {
    return (m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline int RawFTSWFormat::GetRunNoSubRunNo(int n)
  {
    return ((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) &
            (RUNNO_MASK | SUBRUNNO_MASK));
  }

  inline int RawFTSWFormat::GetExpNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

}

#endif
