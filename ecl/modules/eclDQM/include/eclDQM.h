/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDQMODULE_H
#define ECLDQMODULE_H

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h> // <- Substitution of HistoModule.h
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include <framework/core/Module.h>
#include <vector>
#include "TH1F.h"

namespace Belle2 {

  /**
   * This module is for ECL Data Quality Monitor.
   */
  class ECLDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    //! Constructor
    ECLDQMModule();

    //! Destructor
    virtual ~ECLDQMModule();

    //! Initialize the module
    virtual void initialize();
    //! Call when a run begins
    virtual void beginRun();
    //! Event processor
    virtual void event();
    //! Call when a run ends
    virtual void endRun();
    //! Terminate
    virtual void terminate();

    //! Function to define histograms
    virtual void defineHisto();

  private:

    //! Module parameters


    //! Histogram: Crystal Cell IDs
    TH1F* h_cid;
    //! Histogram: Crystal Cell IDs above threshold = 2 MeV
    TH1F* h_cid_Thr2MeV;
    //! Histogram: Crystal Cell IDs above threshold = 5 MeV
    TH1F* h_cid_Thr5MeV;
    //! Histogram: Crystal Cell IDs above threshold = 10 MeV
    TH1F* h_cid_Thr10MeV;
    //! Histogram: Energy deposition in event
    TH1F* h_edep;
    //! Histogram: Reconstructed signal time relative to trigger for barrel calorimeter above threshold = 5 MeV
    TH1F* h_time_barrel_Thr5MeV;
    //! Histogram: Reconstructed signal time relative to trigger for endcap calorimeter above threshold = 5 MeV
    TH1F* h_time_endcaps_Thr5MeV;
    //! Histogram: Reconstructed signal time relative to trigger for barrel calorimeter above threshold = 10 MeV
    TH1F* h_time_barrel_Thr10MeV;
    //! Histogram: Reconstructed signal time relative to trigger for endcap calorimeter above threshold = 10 MeV
    TH1F* h_time_endcaps_Thr10MeV;
    //! Histogram: Reconstructed signal time relative to trigger for barrel calorimeter above threshold = 50 MeV
    TH1F* h_time_barrel_Thr50MeV;
    //! Histogram: Reconstructed signal time relative to trigger for endcap calorimeter above threshold = 50 MeV
    TH1F* h_time_endcaps_Thr50MeV;
    //! Histogram: Fit quality flag (0 - good, 1 - large amplitude, 2 - bad chi2)
    TH1F* h_quality;
    //! Minimum limit for timing histogram
    float time_min  = -2060. / 2.032;
    //! Maximum limit for timing histogram
    float time_max  = 2060. / 2.032;

  };
}
#endif
