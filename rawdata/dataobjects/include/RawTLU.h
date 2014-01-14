//+
// File : RawTLU.h
// Description : Module to handle raw data from TLU(for the telescope beam test)
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Jan - 2014
//-

#ifndef RAWTLU_H
#define RAWTLU_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawTLU : public RawDataBlock {
  public:
    //! Default constructor
    RawTLU();

    //! Destructor
    virtual ~RawTLU();


    //!
    int GetNwords(int n);

    //!
    int GetNwordsHeader(int n);

    //!
    int GetNodeID(int n);

    //!
    unsigned int GetEveNo(int n);

    //!
    unsigned int GetMagicTrailer(int n);

    //!
    int GetRunNo(int n);

    //!
    unsigned int GetTLUEventTag(int n);

    //!
    void CheckData(int n, unsigned int prev_evenum, unsigned int* cur_evenum);


    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_NUMEVE_NUMNODES = 2,
      POS_RUN_NO = 3,
      POS_EVE_NO = 4,
      POS_NA_1 = 5,
      POS_NODE_ID = 6,
      POS_NA_2 = 7,
      POS_TLU_EVENTTAG = 8,
      POS_NA_3 = 9,
      POS_TLU_TIME_1 = 10,
      POS_TLU_TIME_2 = 11,
      POS_RSVD_1 = 12,
      POS_MAGIC_1 = 13
    };

    enum {
      SIZE_TLU_PACKET = 14
    };

    enum {
      TLU_MAGIC_TRAILER = 0x7FFF0000
    };

  protected :

    ClassDef(RawTLU, 1);

  };
}

#endif
