/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESERIALIZERPC_H
#define DESERIALIZERPC_H

#include <string>
#include <vector>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <daq/rawdata/modules/DeSerializer.h>
#include <daq/dataflow/EvtSocket.h>

#include <rawdata/dataobjects/RawCOPPER.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerPCModule : public DeSerializerModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerPCModule();
    virtual ~DeSerializerPCModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  protected :
    //! Accept connection
    virtual int Connect();

    //! receive data
    virtual int recvFD(int fd, char* buf, int data_size_byte, int flag);


    //! receive data
    virtual int* recvData(int* delete_flag, int* total_m_size_word, int* num_events_in_sendblock, int* num_nodes_in_sendblock);

    //! attach buffer to RawDataBlock
    virtual void setRecvdBuffer(RawDataBlock* raw_datablk, int* delete_flag);

    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);
    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* exp_copper_0,
                           unsigned int* run_copper_0, unsigned int* subrun_copper_0,
                           unsigned int* eve_copper_0, unsigned int* error_bit_flag);

    void setErrorFlag(unsigned int error_flag, StoreObjPtr<EventMetaData> evtmetadata);

#ifdef NONSTOP
    //! wait for resume
    virtual void waitResume();
#endif


    //! # of connections
    int m_num_connections;

    //! Reciever basf2 Socket
    std::vector<EvtSocketRecv*> m_recv;

    //! Reciever Socket
    std::vector<int> m_socket;

    //! hostname of upstream Data Sources
    std::vector<std::string> m_hostname_from;

    //! port # to connect data sources
    std::vector<int> m_port_from;

    StoreArray<RawFTSW> raw_ftswarray;

    StoreArray<RawCOPPER> rawcprarray;


    //    StoreArray<ReducedRawCOPPER> rawcprarray;
    ///
    PreRawCOPPERFormat_latest m_pre_rawcpr;

    int event_diff;

    unsigned int m_prev_copper_ctr;

    unsigned int m_prev_evenum;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
