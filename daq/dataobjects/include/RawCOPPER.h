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
    virtual void copy(int* bufin, int nwords);

    //! allocate buffer
    virtual int* allocate_buffer(int nwords);

    //! get buffer
    virtual int* buffer();

    //! set buffer
    virtual void buffer(int* bufin, int nwords);

    //! set buffer
    virtual void buffer(int* bufin, int nwords, int malloc_flag);

    //! get header
    virtual int* header(void);
    //! set header
    virtual void header(int*);

    //! get data
    virtual int* data(void);
    //! set
    virtual void data(int nwords, int*);

    //! get data length
    virtual int get_body_nwords();

    //! get data length
    virtual int size();

    //! get COPPER node id from data
    virtual int get_copper_node_id();

    //! get Event number from data
    virtual unsigned int get_coppereve_no();

    //! get subsystem-ID from data
    virtual int get_subsys_id();

    //! get b2l block from data
    virtual int get_num_b2l_block();

    //! get offset to 1st b2l block
    virtual int offset_1st_b2l_wo_rawhdr();

    //! get offset to 2nd b2l block
    virtual int offset_2nd_b2l_wo_rawhdr();

    //! get offset to 3rd b2l block
    virtual int offset_3rd_b2l_wo_rawhdr();

    //! get offset to 4th b2l block
    virtual int offset_4th_b2l_wo_rawhdr();



    RawHeader m_header; //|| do not split the header
    RawTrailer m_trailer; //|| do not split the header

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
      SIZE_B2LHSLB_HEADER = 2
    };
    /* Data Format : B2link HSLB Trailer */
    enum {
      POS_CHKSUM_B2LHSLB = 0,
      SIZE_B2LHSLB_TRAILER = 1
    };


    /* Data Format : B2link FEF Header */
    enum {
      POS_EVEID_B2LFEF = 0,
      POS_EVETIME_B2LFEF = 1,
      SIZE_B2LFEF_HEADER = 2
    };
    /* Data Format : B2link FEF Trailer */
    enum {
      POS_CHKSUM_B2LFEF = 0,
      SIZE_B2LFEF_TRAILER = 1
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

    ClassDef(RawCOPPER, 1);
  };
}

#endif
