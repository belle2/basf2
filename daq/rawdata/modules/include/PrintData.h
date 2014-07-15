//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTDATA_H
#define PRINTDATA_H

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




#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawBPID.h>
#include <rawdata/dataobjects/RawEPID.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <daq/rawdata/modules/DAQConsts.h>
#ifndef REDUCED_RAWCOPPER
#else
//#include <rawdata/dataobjects/ReducedRawCOPPER.h>
#endif

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
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);
    virtual void printFTSWEvent(RawDataBlock* raw_array, int i);
    virtual void printBuffer(int* buf, int nwords);
    virtual void printPXDEvent(RawPXD* raw_pxd);

#ifndef REDUCED_RAWCOPPER
#else
    //    void printReducedCOPPEREvent(ReducedRawCOPPER* reduced_raw_copper, int i);
#endif

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
