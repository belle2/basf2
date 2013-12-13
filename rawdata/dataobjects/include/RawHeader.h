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
#include <stdlib.h>
#include <TObject.h>

#include <framework/datastore/DataStore.h>


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

    void CheckSetBuffer(); //! initialize header
    void CheckGetBuffer(); //! initialize header

    void Initialize(); //! initialize header

    void SetNwords(int nwords);    //! set contents of header
    void SetExpNo(int exp_no);    //! set contents of header
    void SetRunNo(int run_no);    //! set contents of header
    void SetEveNo(unsigned int eve_no);    //! set contents of header
    void SetSubsysId(int subsys_id);    //! set contents of header
    void SetDataType(int data_type);    //! set contents of header
    void SetTruncMask(int trunc_mask);    //! set contents of header
    void SetB2LFEEHdrPart(unsigned int word1, unsigned int word2);   //! set contents of header

    void SetFTSW2Words(int* ftsw_buf);
    void SetFTSW2Words(unsigned int word1, unsigned int word2);
    void SetExpRunNumber(int* exprun_buf);

    void SetOffset1stFINESSE(int offset_1st_FINESSE);    //! set contents of header
    void SetOffset2ndFINESSE(int offset_2nd_FINESSE);    //! set contents of header
    void SetOffset3rdFINESSE(int offset_3rd_FINESSE);    //! set contents of header
    void SetOffset4thFINESSE(int offset_4th_FINESSE);    //! set contents of header
    void SetMagicWordEntireHeader(); //! set magic words;

    int AddNodeInfo(int node_id);

    int GetNwords();  //! get contents of header
    int GetHdrNwords();  //! get contents of header
    int GetExpNo();  //! get contents of header
    int GetRunNo();  //! get contents of header
    unsigned int GetEveNo();  //! get contents of header
    int GetSubsysId();  //! get contents of header
    int GetDataType();  //! get contents of header
    int GetTruncMask();  //! get contents of header

    int GetOffset1stFINESSE();  //! get contents of header
    int GetOffset2ndFINESSE();  //! get contents of header
    int GetOffset3rdFINESSE();  //! get contents of header
    int GetOffset4thFINESSE();  //! get contents of header
    int GetNumNodes();  //! get contents of header
    int GetNodeInfo(int node_no, int* node_id);    //! get contents of header
    unsigned int GetMagicWordEntireHeader();

    enum {
      RAWHEADER_NWORDS = 20
    };


    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_EXP_RUN_NO = 2,
      POS_EVE_NO = 3,
      POS_HSLB_1 = 4,
      POS_HSLB_2 = 5,
      POS_SUBSYS_ID = 6,
      POS_TRUNC_MASK_DATATYPE = 7,
      POS_OFFSET_1ST_FINESSE = 8,
      POS_OFFSET_2ND_FINESSE = 9,
      POS_OFFSET_3RD_FINESSE = 10,
      POS_OFFSET_4TH_FINESSE = 11,
      POS_RESVD_1 = 12,
      POS_RESVD_2 = 13,
      POS_NUM_NODES = 14,
      POS_NODES_1 = 15,
      POS_NODES_2 = 16,
      POS_NODES_3 = 17,
      POS_NODES_4 = 18,
      POS_TERM_HEADER = 19
    };
    /* Data Format : Node info */
    enum {
      NUM_MAX_NODES = 4   /* (NUM_MAX_NODES + 1) slots are available in m_buffer.
         (NUM_MAX_NODES +1 )th slot is filled with -1, when the number of total nodes
         exceeds NUM_MAX_NODES */
    };

    /* Data Format : Magic word */
    enum {
      MAGIC_WORD_TERM_HEADER = 0x7fff0005
    };

  private:

    int* m_buffer; //! do not record buffer ( RawCOPPER includes buffer of RawHeader and RawTrailer )
    //    unsigned int m_header[HEADER_SIZE];

    ClassDef(RawHeader, 1);
  };




}

#endif
