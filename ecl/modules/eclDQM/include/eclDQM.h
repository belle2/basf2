/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * ECL Data Quality Monitor                                               *
 *                                                                        *
 * This module provides histograms for ECL Data Quality Monitoring        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDQMODULE_H
#define ECLDQMODULE_H

// Copied 6 lines below from PXDDQMModule.h
#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

//FRAMEWORK
#include <framework/core/Module.h>

//ROOT
#include "TH1F.h"

namespace Belle2 {


  /**
   * This module is for ECL Data Quality Monitor.
   */
  class ECLDQMModule : public HistoModule {  /**< derived from HistoModule class. */

  public:

    /** Constructor. */
    ECLDQMModule();

    /** Destructor. */
    virtual ~ECLDQMModule();

    /** Initialize the module. */
    virtual void initialize();
    /** Call when a run begins. */
    virtual void beginRun();
    /** Event processor. */
    virtual void event();
    /** Call when a run ends. */
    virtual void endRun();
    /** Terminate. */
    virtual void terminate();

    /** Function to define histograms. */
    virtual void defineHisto();

  private:

    /** Histogram directory in ROOT file. */
    std::string m_histogramDirectoryName;
    /** Upper threshold of energy deposition in event, [GeV]. */
    double m_EnergyUpperThr;

    /** Histogram: Crystal Cell IDs. */
    TH1F* h_cid;
    /** Histogram: Crystal Cell IDs above threshold = 5 MeV.  */
    TH1F* h_cid_Thr5MeV;
    /** Histogram: Crystal Cell IDs above threshold = 10 MeV. */
    TH1F* h_cid_Thr10MeV;
    /** Histogram: Crystal Cell IDs above threshold = 50 MeV.  */
    TH1F* h_cid_Thr50MeV;
    /** Histogram: Energy deposition in event. */
    TH1F* h_edep;
    /** Histogram: Reconstructed signal time for the barrel calorimeter above the threshold = 5 MeV.  */
    TH1F* h_time_barrel_Thr5MeV;
    /** Histogram: Reconstructed signal time for the endcap calorimeter above the threshold = 5 MeV.  */
    TH1F* h_time_endcaps_Thr5MeV;
    /** Histogram: Reconstructed signal time for the barrel calorimeter above the threshold = 10 MeV. */
    TH1F* h_time_barrel_Thr10MeV;
    /** Histogram: Reconstructed signal time for the endcap calorimeter above the threshold = 10 MeV. */
    TH1F* h_time_endcaps_Thr10MeV;
    /** Histogram: Reconstructed signal time for the barrel calorimeter above the threshold = 50 MeV. */
    TH1F* h_time_barrel_Thr50MeV;
    /** Histogram: Reconstructed signal time for the endcap calorimeter above the threshold = 50 MeV. */
    TH1F* h_time_endcaps_Thr50MeV;
    /** Histogram: Fit quality flag (0 - good, 1 - large amplitude, 3 - bad chi2). */
    TH1F* h_quality;
  }; // end Belle2 namespace
}
#endif
