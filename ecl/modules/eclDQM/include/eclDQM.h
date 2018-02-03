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
#include "TH2F.h"


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
    /** Lower threshold of pedestal distribution. */
    int m_PedestalMeanLowerThr;
    /** Upper threshold of pedestal distribution. */
    int m_PedestalMeanUpperThr;
    /** Upper threshold of pedestal rms error distribution. */
    double m_PedestalRmsUpperThr;
    /** WF sampling points for digit array.   */
    int m_DspArray[8736][31];
    /** Pedestal average values.   */
    int m_PedestalMean[8736];
    /** Pedestal rms error values.    */
    int m_PedestalRms[8736];

    /** Histogram: Crystal Cell IDs w/o software threshold.  */
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
    /** Histogram: Number of hits in each event w/o software threshold.  */
    TH1F* h_ncev;
    /** Histogram: Number of hits in each event above the treshold = 10 MeV.  */
    TH1F* h_ncev_Thr10MeV;
    /** Histogram: Trigger tag flag #1. */
    TH1F* h_trigtag1;
    /** Histogram: Trigger time vs. Trig Cell ID.  */
    TH2F* h_trigtime_trigid;
    /** Histogram: Trigger tag flag #2 vs. Trig Cell ID.   */
    TH2F* h_trigtag2_trigid;
    /** Histogram: Pedestal Average vs. Cell ID.   */
    TH2F* h_pedmean_cellid;
    /** Histogram: Pedestal rms error vs. Cell ID.   */
    TH2F* h_pedrms_cellid;
  }; // end ECL namespace
}; // end Belle2 namespace
#endif
