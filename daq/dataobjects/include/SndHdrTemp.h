/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SNDHDRTEMP_H
#define SNDHDRTEMP_H

#include <TObject.h>

#include "rawdata/dataobjects/RawHeader_latest.h"

namespace Belle2 {

  class SndHdrTemp : public TObject {
    //  class SndHdrTemp {
  public:

    //! Default constructor
    SndHdrTemp();

    //! Constructor using existing pointer to raw data buffer
    SndHdrTemp(int*);
    //! Destructor
    ~SndHdrTemp();

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

    RawHeader_latest tmp_header; //!

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

    ClassDef(SndHdrTemp, 1);
  };




}

#endif
