//+
// File : PrintData.h
// Description : Read RawCOPPER objects and shows their hex dump
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTDATATEMPLATE_H
#define PRINTDATATEMPLATE_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>

//#include <daq/dataflow/EvtSocket.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>




#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/dataobjects/RawTRG.h>

//#include <daq/rawdata/modules/DAQConsts.h>

/* #include <daq/dataobjects/SendHeader.h> */
/* #include <daq/dataobjects/SendTrailer.h> */


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>



namespace Belle2 {

  /*! Module to get data from DataStore and send it to another network node */

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

    //! print the contents of a RawCOPPER event
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);

    //! print the contents of a RawFTSW event
    virtual void printFTSWEvent(RawDataBlock* raw_array, int i);

    //! print a buffer
    virtual void printBuffer(int* buf, int nwords);

    //! print a PXD event
    virtual void printPXDEvent(RawPXD* raw_pxd);


  protected :
    //!Compression parameter
    int m_compressionLevel;

    //! No. of sent events
    int n_basf2evt;

    //! # of FTSWs
    int m_nftsw;

    //! # of COPPERs
    int m_ncpr;

    //! counter
    int m_print_cnt;

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
