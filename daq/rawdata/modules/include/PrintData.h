//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/rawdata/modules/DAQConsts.h>

#include <daq/dataobjects/RawCOPPER.h>
#include <daq/dataobjects/RawSVD.h>
#include <daq/dataobjects/RawCDC.h>
#include <daq/dataobjects/RawBPID.h>
#include <daq/dataobjects/RawEPID.h>
#include <daq/dataobjects/RawECL.h>
#include <daq/dataobjects/RawKLM.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>



namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class PrintDataModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PrintDataModule();
    virtual ~PrintDataModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();
    virtual void VerifyCheckSum(int* buf);
    virtual void PrintEvent(RawCOPPER* raw_array);


    // Data members

  protected :

    // Event Meta Data
    //    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! data
    //    StoreObjPtr<RawCOPPER> m_rawcopper;


    //! size of buffer for one event (word)
    int BUF_SIZE_WORD;

    //!Compression parameter
    int m_compressionLevel;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int n_basf2evt;

    int* m_buffer;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
