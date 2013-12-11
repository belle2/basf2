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


#ifdef READ_OLD_B2LFEE_FORMAT_FILE
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_NUMEVE_NUMNODES = 2,
      POS_EXP_RUN_NO = 3,
      POS_EVE_NO = 4,
      POS_NODE_ID = 5,
      POS_FTSW_1 = 6,
      POS_FTSW_2 = 7,
      POS_FTSW_3 = 8,
      POS_FTSW_4 = 9,
      POS_RSVD_1 = 10,
      POS_MAGIC_1 = 11
    };
#else
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
      POS_FTSW_1 = 8,
      POS_FTSW_2 = 9,
      POS_FTSW_3 = 10,
      POS_FTSW_4 = 11,
      POS_RSVD_1 = 12,
      POS_MAGIC_1 = 13
    };
#endif

  protected :

    ClassDef(RawFTSW, 2);
    // ver.2 Remove m_FTSW_header and introduce a new data format on Nov. 20, 2013
  };
}

#endif
