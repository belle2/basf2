//+
// File : SendHeader.h
// Description : Module to handle SendHeader attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef SENDHEADER_H
#define SENDHEADER_H

#include <string.h>
#include <framework/datastore/DataStore.h>
#include <TObject.h>
#include <daq/rawdata/modules/DAQConsts.h>

#ifdef REDUCED_RAWCOPPER
#include "rawdata/dataobjects/RawHeader_latest.h"
#else
#include "rawdata/dataobjects/RawHeader.h"
#endif



namespace Belle2 {

  class SendHeader : public TObject {
  public:

    //! Default constructor
    SendHeader();

    //! Constructor using existing pointer to raw data buffer
    SendHeader(int*);
    //! Destructor
    ~SendHeader();

    //! Get Header contents
    int* GetBuffer(void);

    //! set buffer
    void SetBuffer(int* hdr);

    void Initialize(); //! initialize Header

    void SetNwords(int total_data_nwords);    //! set contents of Header

    void SetNumEventsinPacket(int num_events);
    void SetNumNodesinPacket(int num_nodes);
    void SetEventNumber(int eve_num);
    void SetNodeID(int node_id);

    void SetExpRunWord(unsigned int exp_run);


    void SetRunNum(int run_num);   //!
    void SetSubRunNum(int subrun_num);  //!
    void SetExpNum(int exp_num);   //!

    int GetTotalNwords();  //! get contents of Header
    int GetHdrNwords();  //! get contents of Header
    int GetNumEventsinPacket();
    int GetNumNodesinPacket();
    int GetEventNumber();
    int GetNodeID();

    int GetRunNum(); //!
    int GetSubRunNum(); //!
    int GetExpNum(); //!
    int GetRunNumSubRunNum(); //!

#ifdef REDUCED_RAWCOPPER
    RawHeader_latest tmp_header; //!
#endif

    /* Data Format : Node info */
    enum {
      SENDHDR_NWORDS = 6
    };

  private:

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_NUM_EVE_NUM_NODES = 2,
      POS_EXP_RUN_NUM = 3,
      POS_EVE_NUM = 4,
      POS_NODE_ID = 5
    };



    int m_buffer[ SENDHDR_NWORDS ];

    ClassDef(SendHeader, 1);
  };




}

#endif
