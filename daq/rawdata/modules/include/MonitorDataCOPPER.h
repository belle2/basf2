#ifndef MONITOR_DATACOPPER_H
#define MONITOR_DATACOPPER_H
//+
// File : MonitorDataCOPPER.h
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
#include <sys/time.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class MonitorDataCOPPERModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    MonitorDataCOPPERModule();
    virtual ~MonitorDataCOPPERModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Histogram definition
    virtual void defineHisto();
    virtual double getTimeSec();
    // Data members

  private:

    //! No. of sent events
    int m_loop;

    int* m_buffer;

    TH1* h_size;
    TH1* h_nevt;
    TH1* h_rate;
    TH1* h_hslb_size[4];
    TH1* h_hslb_nevt;
    TH1* h_hslb_rate;

    int m_nevt;
    int m_prev_nevt;

    double m_start_time;
    double m_prev_time;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
