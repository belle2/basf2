#ifndef DAQMONITOR_H
#define DAQMONITOR_H
//+
// File : DAQMonitor.h
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
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawFTSW.h>

#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DAQMonitorModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DAQMonitorModule();
    virtual ~DAQMonitorModule();

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
    TH1F* h_pxdsize;
    TH1F* h_svdsize;
    TH1F* h_cdcsize;
    TH1F* h_topsize;
    TH1F* h_arichsize;
    TH1F* h_eclsize;
    TH1F* h_klmsize;
    TH1F* h_trgsize;

    TH1F* h_nevt;
    TH1F* h_size;
    TH1F* h_hltsize;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
