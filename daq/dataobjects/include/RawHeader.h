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

#include <stdlib.h>

//#define HEADER_SIZE 16



namespace Belle2 {

  class RawHeader : public TObject {
  public:
    //! Default constructor
    RawHeader();

    //! Constructor using existing pointer to raw data buffer
    RawHeader(int*);
    //! Destructor
    ~RawHeader();

    //! Get header contents
    int* GetBuffer();

    //! set buffer
    void SetBuffer(int* hdr);

    void CheckBuffer(); //! initialize header

    void Initialize(); //! initialize header

    void SetNwords(int nwords);    //! set contents of header
    void SetExpNo(int exp_no);    //! set contents of header
    void SetRunNo(int run_no);    //! set contents of header
    void SetEveNo(unsigned int eve_no);    //! set contents of header
    void SetSubsysId(int subsys_id);    //! set contents of header
    void SetDataType(int data_type);    //! set contents of header
    void SetTruncMask(int trunc_mask);    //! set contents of header
    void SetNumB2lBlock(int num_b2l_block);    //! set contents of header
    void SetOffset1stB2l(int offset_1st_b2l);    //! set contents of header
    void SetOffset2ndB2l(int offset_2nd_b2l);    //! set contents of header
    void SetOffset3rdB2l(int offset_3rd_b2l);    //! set contents of header
    void SetOffset4thB2l(int offset_4th_b2l);    //! set contents of header
    int AddNodeInfo(int node_id);

    int GetNwords();  //! get contents of header
    int GetHdrNwords();  //! get contents of header
    int GetExpNo();  //! get contents of header
    int GetRunNo();  //! get contents of header
    unsigned int GetEveNo();  //! get contents of header
    int GetSubsysId();  //! get contents of header
    int GetDataType();  //! get contents of header
    int GetTruncMask();  //! get contents of header
    int GetNumB2lBlock();  //! get contents of header
    int GetOffset1stB2l();  //! get contents of header
    int GetOffset2ndB2l();  //! get contents of header
    int GetOffset3rdB2l();  //! get contents of header
    int GetOffset4thB2l();  //! get contents of header
    int GetNumNodes();  //! get contents of header
    int GetNodeInfo(int node_no, int* node_id);    //! get contents of header
    unsigned int GetMagicWordFixedPart();
    unsigned int GetMagicWordEntireHeader();

    enum {
      RAWHEADER_NWORDS = 23
    };


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
      POS_NODES_0 = 17,
      POS_NODES_1 = 18,
      POS_NODES_2 = 19,
      POS_NODES_3 = 20,
      POS_NODES_4 = 21,
      POS_TERM_HEADER = 22
    };

  private:
    /* Data Format : Node info */
    enum {
      NUM_MAX_NODES = 5   /* (NUM_MAX_NODES + 1) slots are available in m_buffer.
         (NUM_MAX_NODES +1 )th slot is filled with -1, when the number of total nodes
         exceeds NUM_MAX_NODES */
    };

    /* Data Format : Magic word */
    enum {
      MAGIC_WORD_TERM_FIXED_PART = 0x7fff0004,
      MAGIC_WORD_TERM_HEADER = 0x7fff0005
    };



    int* m_buffer;
    //    unsigned int m_header[HEADER_SIZE];

    ClassDef(RawHeader, 1);
  };




}

#endif
