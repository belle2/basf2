//+
// File : DAQMonitor.h
// Description : Module to monitor raw data accumulating histos
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date :  5 - Sep - 2013
//-

#pragma once

#include <framework/core/HistoModule.h>

#include "TH1F.h"

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

    /** No. of sent events */
    int m_nevt;

    /** Histogram for PXD data size */
    TH1F* h_pxdsize;
    /** Histogram for SVD data size */
    TH1F* h_svdsize;
    /** Histogram for CDC data size */
    TH1F* h_cdcsize;
    /** Histogram for TOP data size */
    TH1F* h_topsize;
    /** Histogram for ARICH data size */
    TH1F* h_arichsize;
    /** Histogram for ECL data size */
    TH1F* h_eclsize;
    /** Histogram for KLM data size */
    TH1F* h_klmsize;
    /** Histogram for TRG data size */
    TH1F* h_trgsize;

    /** Histogram for total number of events */
    TH1F* h_nevt;
    /** Histogram for total data size */
    TH1F* h_size;
    /** Histogram for HLT data size */
    TH1F* h_hltsize;

  };

} // end namespace Belle2
