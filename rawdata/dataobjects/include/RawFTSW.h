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
#include <rawdata/dataobjects/RawDataBlock.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawFTSW : public RawDataBlock {
  public:
    //! Default constructor
    RawFTSW();
    //! Constructor using existing pointer to raw data buffer
    //RawFTSW(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSW();

    //!
    int GetNwords(int n);

    //!
    int GetNwordsHeader(int n);

    //!
    int GetFTSWNodeID(int n);

    //!
    unsigned int GetEveNo(int n);

    //!
    unsigned int GetTTUtime(int n);

    //!
    unsigned int GetTTCtimeTRGType(int n);

    //!
    unsigned int GetMagicTrailer(int n);

    //!
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   int prev_run_no, int* cur_run_no);

    //!
    int GetRunNo(int n);

    //!
    int GetSubRunNo(int n);

    //!
    int GetRunNoSubRunNo(int n);


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
      POS_TT_CTIME_TYPE = 8,
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
      SUBRUNNO_MASK = 0x000000FF
    };

    enum {
      SIZE_FTSW_PACKET = 14
    };
    //#endif

    enum {
      FTSW_MAGIC_TRAILER = 0x7FFF0000
    };

  protected :

    ClassDef(RawFTSW, 2);
    // ver.2 Remove m_FTSW_header and introduce a new data format on Nov. 20, 2013
  };

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
}

#endif
