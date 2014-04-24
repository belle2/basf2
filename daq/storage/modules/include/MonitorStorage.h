#ifndef MONITOR_STORAGE_H
#define MONITOR_STORAGE_H
//+
// File : MonitorStorage.h
// Description : Module to monitor data storage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date :  5 - Sep - 2013
//-

#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/time.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawCOPPER.h>

//#include <daq/rawdata/modules/DAQConsts.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <daq/storage/storager_data.h>

#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class MonitorStorageModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    MonitorStorageModule();
    virtual ~MonitorStorageModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Histogram definition
    virtual void defineHisto();

  public:
    static storager_data& getData() { return g_data; }

  private:
    static storager_data g_data;

  private:

    TH1* m_h_runinfo;
    //TH1* m_h_datasize;
    //TH1* m_h_flowrate;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
