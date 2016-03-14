//+
// File : DesSerCOPPER.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 4 - Sep - 2014
//-

#ifndef DESSERCOPPER_H
#define DESSERCOPPER_H

#include <daq/rawdata/DesSer.h>


////////////////////////////////////////////
// From Serializer.cc
////////////////////////////////////////////


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DesSerCOPPER : public DesSer {

    // Public functions
  public:

    //! Constructor / Destructor
    DesSerCOPPER(string host_recv, int port_recv, string host_send, int port_send, int shmflag,
                 const std::string& nodename, int nodeid, int finesse_bitflag);
    //    DesSerCOPPER();
    virtual ~DesSerCOPPER();

    //! Module functions to be called from event process
    void DataAcquisition();

    //! Read one event from COPPER
    int* readOneEventFromCOPPERFIFO(const int entry, int* delete_flag, int* m_size_word);

    //! open COPPER
    void openCOPPER();

    //! Module functions to be called from main process
    void initializeCOPPER();

    //! receive data
    int readFD(int fd, char* buf, int data_size_byte, int delete_flag);

    //! file descripter for COPPER
    int m_cpr_fd;

    //! bit flag of FINESSE slots to use
    int m_use_slot;

    //! bit flag of FINESSE slots to use
    int m_finesse_bit_flag;

  protected :


#ifdef NONSTOP
    //! for nonstop DAQ
    void waitResume();

    //! for nonstop DAQ
    void resumeRun();
#endif
  };

} // end namespace Belle2

#endif // DESSERCOPPER_H
