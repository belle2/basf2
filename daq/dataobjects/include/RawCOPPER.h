//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWCOPPER_H
#define RAWCOPPER_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/RawHeader.h>
#include <daq/dataobjects/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawCOPPER : public TObject {
  public:
    //! Default constructor
    RawCOPPER();
    //! Constructor using existing pointer to raw data buffer
    RawCOPPER(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPER();

    //! copy rawdata into internal buffer
    //    virtual void Copy(int* bufin, int nwords);

    //! allocate buffer


    //! get buffer
    virtual int* GetBuffer();

    //! set buffer
    virtual void SetBuffer(int* bufin, int nwords, int malloc_flag);

    //! get data length
    virtual int Size();

    //! get data length
    virtual int GetBodyNwords();

    /*     //! get data length */
    /*     virtual RawHeader* GetRawHeader(); */

    /*     //! get data length */
    /*     virtual RawTrailer* GetRawTrailer(); */

    //! get buffer pointer of rawcopper header
    virtual int* GetRawHdrBufPtr();

    //! get buffer pointer of rawcopper trailer
    virtual int* GetRawTrlBufPtr();

    //! get COPPER node id from data
    virtual int GetCOPPERNodeId();

    //! get Event number from data
    virtual unsigned int GetCOPPEREveNo();

    //! get Event number from data
    virtual unsigned int GetB2LFEEHdr1();

    //! get Event number from data
    virtual unsigned int GetB2LFEEHdr2();

    //! get subsystem-ID from data
    virtual int GetSubsysId();

    //! get b2l block from data
    virtual int GetNumB2lBlock();


    //! get # of offset words for FEE slot A buffer position
    int GetOffset1stB2l();

    //! get # of offset words for FEE slot A buffer position
    int GetOffset2ndB2l();

    //! get # of offset words for FEE slot A buffer position
    int GetOffset3rdB2l();

    //! get # of offset words for FEE slot A buffer position
    int GetOffset4thB2l();

    //! get FEE buffer pointer for slot A
    int* Get1stFEEBuffer();

    //! get FEE buffer pointer for slot B
    int* Get2ndFEEBuffer();

    //! get FEE buffer pointer for slot C
    int* Get3rdFEEBuffer();

    //! get FEE buffer pointer for slot D
    int* Get4thFEEBuffer();


    /* Data Format : COPPER header*/
    enum {
      POS_MAGIC_COPPER_1 = 0,
      POS_EVE_NUM_COPPER = 1,
      POS_SUBSYSTEM_ID = 2,
      POS_CRATE_ID = 3,
      POS_SLOT_ID = 4,
      POS_MAGIC_COPPER_2 = 7,
      POS_DATA_LENGTH = 8,
      POS_CH_A_DATA_LENGTH = 9,
      POS_CH_B_DATA_LENGTH = 10,
      POS_CH_C_DATA_LENGTH = 11,
      POS_CH_D_DATA_LENGTH = 12,
      SIZE_COPPER_HEADER = 13
    };

    /* Data Format : B2link HSLB Header */
    enum {
      POS_MAGIC_B2LHSLB = 0,
      POS_EVE_CNT_B2LHSLB = 1,
      //      SIZE_B2LHSLB_HEADER = 2
      SIZE_B2LHSLB_HEADER = 0 // As of Aug.22,2013, the header size is one word. It should be 2 in future.
    };
    /* Data Format : B2link HSLB Trailer */
    enum {
      POS_CHKSUM_B2LHSLB = 0,
      SIZE_B2LHSLB_TRAILER = 1
    };


    /* Data Format : B2link FEE Header */
    enum {
      POS_EVEID_B2LFEE = 0,
      POS_EVETIME_B2LFEE = 1,
      //      SIZE_B2LFEE_HEADER = 2
      SIZE_B2LFEE_HEADER = 1  // As of Aug.22,2013, the header size is one word. It should be 2 in future.
    };


    /* Data Format : B2link FEE Trailer */
    enum {
      POS_CHKSUM_B2LFEE = 0,
      SIZE_B2LFEE_TRAILER = 1
    };


    /* Data Format : COPPER Trailer*/
    enum {
      POS_MAGIC_COPPER_3 = 0,
      POS_CHKSUM_COPPER = 1,
      POS_MAGIC_COPPER_4 = 2
    };


  private:


    int m_nwords;
    int* m_buffer; //[m_nwords]
    bool m_allocated;

    RawHeader tmp_header;  //! Not record
    RawTrailer tmp_trailer; //! Not record


    ClassDef(RawCOPPER, 1);
  };
}

#endif
