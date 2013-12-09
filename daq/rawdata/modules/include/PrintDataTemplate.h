//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTDATATEMPLATE_H
#define PRINTDATATEMPLATE_H

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

#include <rawdata/RawDataBlock.h>
#include <rawdata/RawFTSW.h>
#include <rawdata/RawCOPPER.h>
#include <rawdata/RawSVD.h>
#include <rawdata/RawCDC.h>
#include <rawdata/RawBPID.h>
#include <rawdata/RawEPID.h>
#include <rawdata/RawECL.h>
#include <rawdata/RawKLM.h>

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

  class PrintDataTemplateModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PrintDataTemplateModule();
    virtual ~PrintDataTemplateModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();
    virtual void PrintCOPPEREvent(RawCOPPER* raw_array, int i);
    virtual void PrintFTSWEvent(RawDataBlock* raw_array, int i);
    virtual void PrintData(int* buf, int nwords);

  protected :
    //!Compression parameter
    int m_compressionLevel;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int n_basf2evt;

    int m_nftsw;

    int m_ncpr;

    //!
    int m_print_cnt;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
