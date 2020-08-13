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
    ~DAQMonitorModule();

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    //! Histogram definition
    void defineHisto() override final;

    // Data members

  private:

    /** No. of sent events */
    int m_nevt;

    /** Histogram for PXD data size */
    TH1F* h_pxdsize = NULL;
    /** Histogram for SVD data size */
    TH1F* h_svdsize = NULL;
    /** Histogram for CDC data size */
    TH1F* h_cdcsize = NULL;
    /** Histogram for TOP data size */
    TH1F* h_topsize = NULL;
    /** Histogram for ARICH data size */
    TH1F* h_arichsize = NULL;
    /** Histogram for ECL data size */
    TH1F* h_eclsize = NULL;
    /** Histogram for KLM data size */
    TH1F* h_klmsize = NULL;
    /** Histogram for TRG data size */
    TH1F* h_trgsize = NULL;

    /** Histogram for total number of events */
    TH1F* h_nevt = NULL;
    /** Histogram for total data size */
    TH1F* h_size = NULL;
    /** Histogram for HLT data size */
    TH1F* h_hltsize = NULL;

  };

} // end namespace Belle2
