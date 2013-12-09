#ifndef MONITOR_DATA_H
#define MONITOR_DATA_H
//+
// File : MonitorData.h
// Description : Module to monitor raw data accumulating histos
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

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/RawCOPPER.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class MonitorDataModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    MonitorDataModule();
    virtual ~MonitorDataModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Histogram definition
    virtual void defineHisto();

    // Data members

  private:

    //! No. of sent events
    int m_nevt;
    int* m_buffer;

    //! Histograms
    TH1F* h_ncpr;
    TH1F* h_nevt;
    TH1F* h_size;
    TH2F* h_size2d;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
