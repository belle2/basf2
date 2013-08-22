//+
// File : ROPCHeader.h
// Description : Module to handle ROPCHeader attached on ROPC
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 16 - Aug - 2013
//-

#ifndef ROPCHEADER_H
#define ROPCHEADER_H

#include <string.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>

#include <stdlib.h>

//#define HEADER_SIZE 16



namespace Belle2 {

  class ROPCHeader : public TObject {
  public:
    //! Default constructor
    ROPCHeader();

    //! Constructor using existing pointer to raw data buffer
    ROPCHeader(int*);
    //! Destructor
    ~ROPCHeader();

    //! Get header contents
    int* GetBuffer();

    //! set buffer
    void SetBuffer(int* hdr);
    void CheckBuffer(); //! set contents of header
    void Initialize(); //! initialize header

    void SetNwords(int nwords);    //! set contents of header
    void SetExpNo(int exp_no);    //! set contents of header
    void SetRunNo(int run_no);    //! set contents of header
    void SetEveNo(unsigned int eve_no);    //! set contents of header
    void SetROPCNodeId(int ropc_node_id);    //! set contents of header


    int GetNwords();  //! get contents of header
    int GetHdrNwords();  //! get contents of header
    int GetExpNo();  //! get contents of header
    int GetRunNo();  //! get contents of header
    unsigned int GetEveNo();  //! get contents of header
    int GetROPCNodeId();  //! get contents of header

    enum {
      ROPCHEADER_NWORDS = 6
    };


    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_EXP_RUN_NO = 2,
      POS_EVE_NO = 3,
      POS_ROPC_NODE_ID = 4,
      POS_RESVD_1 = 5,
    };


  private:

    int* m_buffer; //! do not record buffer ( RawROPC includes buffer of RawHeader and RawTrailer )
    //    unsigned int m_header[HEADER_SIZE];

    ClassDef(ROPCHeader, 1);
  };




}

#endif
