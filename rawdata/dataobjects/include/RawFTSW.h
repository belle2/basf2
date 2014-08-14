//+
// File : RawFTSW.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWFTSW_H
#define RAWFTSW_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  /**
   * The Raw FTSW class.
   *
   * This class stores the RAW data containing FTSW data(event #, trg time ).
   */
  class RawFTSW : public RawDataBlock {
  public:
    //! Default constructor
    RawFTSW();
    //! Constructor using existing pointer to raw data buffer
    //RawFTSW(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSW();

    //! Get # of words in this buffer
    int GetNwords(int n);

    //! Get # of words of header
    int GetNwordsHeader(int n);

    //! Get Node # ( should be "TTD " )
    int GetFTSWNodeID(int n);

    //! Get event #
    unsigned int GetEveNo(int n);

    //! Get a word containing ctime and trigger type info
    unsigned int GetTTCtimeTRGType(int n);

    //! get unixtime of the trigger
    unsigned int GetTTUtime(int n);

    //! Get ctime of the trigger
    int GetTTCtime(int n);

    //! Get timeval from ctime and utime
    void GetTTTimeVal(int n, struct timeval* tv);

    //! Get magic number for data corruption check
    unsigned int GetMagicTrailer(int n);



    //! check the data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   int prev_run_no, int* cur_run_no);

    //! Get run #
    int GetRunNo(int n);

    //! Get exp and run+subrun word
    unsigned int GetExpRunWord(int n);

    //! Get subrun #
    int GetSubRunNo(int n);

    //! get a word cotaining run # and subrun #
    int GetRunNoSubRunNo(int n);

    //! Get Exp #
    int GetExpNo(int n);

    //! DESY test only
    int Get15bitTLUTag(int n);


    /* #ifdef READ_OLD_B2LFEE_FORMAT_FILE */
    /*     Warning!!! You are about to use the old format!!!! // it causes compile error */
    /*     enum { */
    /*       POS_NWORDS = 0, */
    /*       POS_HDR_NWORDS = 1, */
    /*       POS_NUMEVE_NUMNODES = 2, */
    /*       POS_EXP_RUN_NO = 3, */
    /*       POS_EVE_NO = 4, */
    /*       POS_NODE_ID = 5, */
    /*       POS_FTSW_1 = 6, */
    /*       POS_FTSW_2 = 7, */
    /*       POS_FTSW_3 = 8, */
    /*       POS_FTSW_4 = 9, */
    /*       POS_RSVD_1 = 10, */
    /*       POS_MAGIC_1 = 11 */
    /*     }; */

    /*     enum { */
    /*       SIZE_FTSW_PACKET = 12 */
    /*     }; */
    /* #else */
    //

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
      TTCTIME_SHIFT = 4
    };

    enum {
      SIZE_FTSW_PACKET = 14
    };
    //#endif

    enum {
      FTSW_MAGIC_TRAILER = 0x7FFF0000
    };

  protected :
    /// To derive from TObject
    /// ver.2 Remove m_FTSW_header and introduce a new data format on Nov. 20, 2013
    ClassDef(RawFTSW, 2);

  };

  inline unsigned int RawFTSW::GetExpRunWord(int n)
  {
    return (unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]);
  }

  inline int RawFTSW::GetRunNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawFTSW::GetSubRunNo(int n)
  {
    return (m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline int RawFTSW::GetRunNoSubRunNo(int n)
  {
    return ((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) &
            (RUNNO_MASK | SUBRUNNO_MASK));
  }

  inline int RawFTSW::GetExpNo(int n)
  {
    return (((unsigned int)(m_buffer[ GetBufferPos(n) + POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

}

#endif
