//+
// File : DeSerializerPC.cc
// Description : Module to read COPPER FIFO and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef DESERIALIZERCOPPER_H
#define DESERIALIZERCOPPER_H

#include <string>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "copper.h"

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/dataobjects/EventMetaData.h>
#include <daq/dataobjects/RawCDC.h>
#include <daq/rawdata/modules/DAQConsts.h>

#include <daq/rawdata/modules/DeSerializer.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerCOPPERModule : public  DeSerializerModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerCOPPERModule();
    virtual ~DeSerializerCOPPERModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    // Data members
  private:

    virtual void FillNewRawCOPPERHeader(RawCOPPER* raw_copper);
    virtual int* ReadCOPPERFIFO(const int entry, int* malloc_flag, int* m_size_word);
    virtual void* OpenCOPPER();

    //! receive data
    virtual int Read(int fd, char* buf, int data_size_byte);

    // Parallel processing parameters


    //! COPPER file descripter
    int cpr_fd;

    int use_slot;


  };

} // end namespace Belle2

#endif
