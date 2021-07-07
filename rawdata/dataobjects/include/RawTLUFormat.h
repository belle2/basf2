/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWTLUFORMAT_H
#define RAWTLUFORMAT_H
#include <rawdata/dataobjects/RawDataBlockFormat.h>

namespace Belle2 {

  /**
   * The Raw TLU class
   * Class for data from DAQ PC for TLU(Trigger Logic Unit)
   * It is supposed to be used only in the DESY beam test
   */

  class RawTLUFormat : public RawDataBlockFormat {
  public:
    //! Default constructor
    RawTLUFormat();

    //! Destructor
    virtual ~RawTLUFormat();

    /*     //! Get # of words in this data packet */
    /*     int GetNwords(int n); */

    //!Get the size of the header
    int GetNwordsHeader(int n);

    //! Get Node ID
    unsigned int GetNodeID(int n);

    //! Get Event #
    unsigned int GetEveNo(int n);

    //! Get Magic Trailer #
    unsigned int GetMagicTrailer(int n);

    //! Get Run #
    int GetRunNo(int n);

    //! Get TLU event tag
    unsigned int GetTLUEventTag(int n);

    //! check data contents
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
  };
}

#endif
