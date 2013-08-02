//+
// File : RawHeader.h
// Description : Module to handle RawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWHEADER_H
#define RAWHEADER_H

#include <string.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>

//#define HEADER_SIZE 16
#define HEADER_SIZE (18+2*10)


namespace Belle2 {

  class RawHeader : public TObject {
  public:
    //! Default constructor
    RawHeader();

    //! Constructor using existing pointer to raw data buffer
    RawHeader(unsigned int*);
    //! Destructor
    ~RawHeader();

    //! Get header contents
    unsigned int* header(void);

    //! set buffer
    void header(unsigned int* hdr);

    void initialize(); //! initialize header

    void set_nwords(int nwords);    //! set contents of header
    void set_hdr_nwords();  //! set contents of header
    void set_exp_no(int exp_no);    //! set contents of header
    void set_run_no(int run_no);    //! set contents of header
    void set_eve_no(unsigned int eve_no);    //! set contents of header
    void set_subsys_id(int subsys_id);    //! set contents of header
    void set_data_type(int data_type);    //! set contents of header
    void set_trunc_mask(int trunc_mask);    //! set contents of header
    void set_num_b2l_block(int num_b2l_block);    //! set contents of header
    void set_offset_1st_b2l(int offset_1st_b2l);    //! set contents of header
    void set_offset_2nd_b2l(int offset_2nd_b2l);    //! set contents of header
    void set_offset_3rd_b2l(int offset_3rd_b2l);    //! set contents of header
    void set_offset_4th_b2l(int offset_4th_b2l);    //! set contents of header
    int add_nodeinfo_and_update_hdrsize(int node_id, int eve_no);

    int get_nwords();  //! get contents of header
    int get_hdr_nwords();  //! get contents of header
    int get_exp_no();  //! get contents of header
    int get_run_no();  //! get contents of header
    unsigned int get_eve_no();  //! get contents of header
    int get_subsys_id();  //! get contents of header
    int get_data_type();  //! get contents of header
    int get_trunc_mask();  //! get contents of header
    int get_num_b2l_block();  //! get contents of header
    int get_offset_1st_b2l();  //! get contents of header
    int get_offset_2nd_b2l();  //! get contents of header
    int get_offset_3rd_b2l();  //! get contents of header
    int get_offset_4th_b2l();  //! get contents of header
    int get_num_nodes();  //! get contents of header
    int get_node_info(int node_no, int* node_id, int* eve_no);    //! get contents of header
    unsigned int get_magic_word_fixed_part();
    unsigned int get_magic_word_entire_header();

  private:

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_EXP_RUN_NO = 2,
      POS_EVE_NO = 3,
      POS_RESERVED_0 = 4,
      POS_SUBSYS_ID = 5,
      POS_DATA_TYPE = 6,
      POS_TRUNC_MASK = 7,
      POS_NUM_B2L_BLOCK = 8,
      POS_OFFSET_1ST_B2L = 9,
      POS_OFFSET_2ND_B2L = 10,
      POS_OFFSET_3RD_B2L = 11,
      POS_OFFSET_4TH_B2L = 12,
      POS_RESVD_1 = 13,
      POS_RESVD_2 = 14,
      POS_TERM_FIXED_PART = 15,
      POS_NUM_NODES = 16,
      MIN_POS_TERM_HEADER = 17
    };

    /* Data Format : Node info */
    enum {
      POS_NODE_ID = 0,
      POS_NODE_EVE_NO = 1,
      SIZE_NODE_INFO = 2,
      NUM_MAX_NODES = 9   /* (NUM_MAX_NODES + 1) slots are available in m_buffer.
         (NUM_MAX_NODES +1 )th slot is filled with -1, when the number of total nodes
         exceeds NUM_MAX_NODES */
    };

    /* Data Format : Magic word */
    enum {
      MAGIC_WORD_TERM_FIXED_PART = 0x7fff0004,
      MAGIC_WORD_TERM_HEADER = 0x7fff0005
    };

    unsigned int m_header[HEADER_SIZE];

    ClassDef(RawHeader, 1);
  };




}

#endif
