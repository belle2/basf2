/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESSERPREPC_H
#define DESSERPREPC_H
#include <daq/rawdata/DesSer.h>

////////////////////////////////////////////
// From Serializer.cc
////////////////////////////////////////////


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DesSerPrePC : public DesSer {

    // Public functions
  public:

    //! Constructor / Destructor
    DesSerPrePC(std::string host_recv, int port_recv, std::string host_send, int port_send, int shmflag,
                const std::string& nodename, int nodeid);
    //    DesSerPrePC();
    virtual ~DesSerPrePC();

    //! Module functions to be called from event process
    void DataAcquisition();

  protected :
    //! Accept connection
    int Connect();

    //! receive data
    int recvFD(int fd, char* buf, int data_size_byte, int flag);

    //! receive data
    int* recvData(int* delete_flag, int* total_m_size_word, int* num_events_in_sendblock, int* num_nodes_in_sendblock);

    //! attach buffer to RawDataBlock
    void setRecvdBuffer(RawDataBlockFormat* raw_datablk, int* delete_flag);

    //! check data contents
    void checkData(RawDataBlockFormat* raw_datablk, unsigned int* eve_copper_0);

#ifdef NONSTOP

    //! for nonstop DAQ
    void waitResume();
#endif

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
