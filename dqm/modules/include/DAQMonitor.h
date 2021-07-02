/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DAQMonitor.h
// Description : Module to monitor raw data accumulating histos
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date :  5 - Sep - 2013
//-

#pragma once

/* Belle 2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>

/* ROOT headers. */
#include <TH1F.h>

namespace Belle2 {

  /** A module for producing general DAQ DQM histograms. */
  class DAQMonitorModule : public HistoModule {

  public:

    /** Constructor. */
    DAQMonitorModule();

    /** Destructor. */
    ~DAQMonitorModule() = default;

    /** Initialize. */
    void initialize() override final;

    /** Begin run. */
    void beginRun() override final;

    /** Event. */
    void event() override final;

    /** Histograms definition. */
    void defineHisto() override final;

  private:

    /** Histogram for total number of events. */
    TH1F* h_nEvt{nullptr};

    /** Histogram for PXD data size. */
    TH1F* h_pxdSize{nullptr};

    /** Histogram for SVD data size. */
    TH1F* h_svdSize{nullptr};

    /** Histogram for CDC data size. */
    TH1F* h_cdcSize{nullptr};

    /** Histogram for TOP data size. */
    TH1F* h_topSize{nullptr};

    /** Histogram for ARICH data size. */
    TH1F* h_arichSize{nullptr};

    /** Histogram for ECL data size. */
    TH1F* h_eclSize{nullptr};

    /** Histogram for KLM data size. */
    TH1F* h_klmSize{nullptr};

    /** Histogram for TRG data size. */
    TH1F* h_trgSize{nullptr};

    /** Histogram for HLT data size. */
    TH1F* h_hltSize{nullptr};

    /** Histogram for total data size. */
    TH1F* h_totalSize{nullptr};

    /** PXD raw data. */
    StoreArray<RawPXD> m_pxdRaw;

    /** SVD raw data. */
    StoreArray<RawSVD> m_svdRaw;

    /** CDC raw data. */
    StoreArray<RawCDC> m_cdcRaw;

    /** TOP raw data. */
    StoreArray<RawTOP> m_topRaw;

    /** ARICH raw data. */
    StoreArray<RawARICH> m_arichRaw;

    /** ECL raw data. */
    StoreArray<RawECL> m_eclRaw;

    /** KLM raw data. */
    StoreArray<RawKLM> m_klmRaw;

    /** TRG raw data. */
    StoreArray<RawTRG> m_trgRaw;

  };

}
