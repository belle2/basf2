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
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/DeSerializer.h>

//#define REDUCED_RAWCOPPER
#ifndef REDUCED_RAWCOPPER
#include <rawdata/dataobjects/RawCOPPER.h>
#else
#include <daq/dataobjects/PreRawCOPPER.h>
#endif


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

#ifndef REDUCED_RAWCOPPER
    //! Fill RawHeader
    virtual void fillNewRawCOPPERHeader(RawCOPPER* raw_copper);
#endif



#ifdef NONSTOP
    //! for nonstop DAQ
    virtual void restartRun();

    //! for nonstop DAQ
    virtual void waitRestart();
#endif

  private:

#ifndef REDUCED_RAWCOPPER
    //! data
    StoreObjPtr<RawCOPPER> m_rawcopper;

    StoreArray<RawCOPPER> rawcprarray;
#else
    //! data
    StoreObjPtr<PreRawCOPPER> m_rawcopper;

    StoreArray<PreRawCOPPER> rawcprarray;
#endif

    StoreArray<RawDataBlock> raw_dblkarray;

    virtual int* readOneEventFromCOPPERFIFO(const int entry, int* malloc_flag, int* m_size_word);
    virtual void openCOPPER();
    //! receive data
    virtual int readFD(int fd, char* buf, int data_size_byte);

    //! Module functions to be called from main process
    virtual void initializeCOPPER();

    // Parallel processing parameters

    //! COPPER file descripter
    int m_cpr_fd;

    //! bit flag for available FINESSE slot
    int m_use_slot;

    //! bit flag for available FINESSE slot
    int m_finesse_bit_flag;



    //! Event # from data
    unsigned int m_prev_ftsweve32;

    //! exp #
    int m_exp;

    //! run #
    int m_run;




  };

} // end namespace Belle2

#endif
