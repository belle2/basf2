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

#pragma once

// Copied 6 lines below from PXDDQMModule.h
#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

//FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

class TH1F;
class TH2F;

namespace Belle2 {

  /**
   * This module is for ECL Data Quality Monitor.
   */
  class ECLDigit;
  class EventMetaData;
  class ECLDsp;
  class ECLTrig;
  class ECLCalDigit;

  class ECLDQMModule : public HistoModule {  /**< derived from HistoModule class. */

  public:

    /** Constructor. */
    ECLDQMModule();

    /** Destructor. */
    virtual ~ECLDQMModule();

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Call when a run ends. */
    virtual void endRun() override;
    /** Terminate. */
    virtual void terminate() override;

    /** Function to define histograms. */
    virtual void defineHisto() override;

  private:
    /** StoreArray ECLDigit */
    StoreArray<ECLDigit> m_ECLDigits;
    /** StoreArray ECLCalDigit */
    StoreArray<ECLCalDigit> m_ECLCalDigits;
    /** StoreArray ECLTrig */
    StoreArray<ECLTrig> m_ECLTrigs;
    /** StoreArray ECLDsp */
    StoreArray<ECLDsp> m_ECLDsps;
    /** StoreArray EventMetaData */
    StoreObjPtr<EventMetaData> m_eventmetadata;

    /** Global event number. */
    int m_iEvent{ -1};
    /** Histogram directory in ROOT file. */
    std::string m_histogramDirectoryName;
    /** Upper threshold of number of hits in event. */
    int m_NHitsUpperThr1;
    /** Upper threshold of number of hits in event (w/ Thr = 10 MeV). */
    int m_NHitsUpperThr2;
    /** Upper threshold of energy deposition in event, [GeV]. */
    double m_EnergyUpperThr;
    /** Lower threshold of pedestal distribution. */
    int m_PedestalMeanLowerThr;
    /** Upper threshold of pedestal distribution. */
    int m_PedestalMeanUpperThr;
    /** Upper threshold of pedestal rms error distribution. */
    double m_PedestalRmsUpperThr;
    /** WF sampling points for digit array.   */
    int m_DspArray[8736][31] = {};
    /** Pedestal average values.   */
    int m_PedestalMean[8736] = {};
    /** Pedestal rms error values.    */
    int m_PedestalRms[8736] = {};

    /** Histogram: Crystal Cell IDs w/o software threshold.  */
    TH1F* h_cid{nullptr};
    /** Histogram: Crystal Cell IDs above threshold = 5 MeV.  */
    TH1F* h_cid_Thr5MeV{nullptr};
    /** Histogram: Crystal Cell IDs above threshold = 10 MeV. */
    TH1F* h_cid_Thr10MeV{nullptr};
    /** Histogram: Crystal Cell IDs above threshold = 50 MeV.  */
    TH1F* h_cid_Thr50MeV{nullptr};
    /** Histogram: Energy deposition in event. */
    TH1F* h_edep{nullptr};
    /** Histogram: Reconstructed signal time for the barrel calorimeter above the threshold = 5 MeV.  */
    TH1F* h_time_barrel_Thr5MeV{nullptr};
    /** Histogram: Reconstructed signal time for the endcap calorimeter above the threshold = 5 MeV.  */
    TH1F* h_time_endcaps_Thr5MeV{nullptr};
    /** Histogram: Reconstructed signal time for the barrel calorimeter above the threshold = 10 MeV. */
    TH1F* h_time_barrel_Thr10MeV{nullptr};
    /** Histogram: Reconstructed signal time for the endcap calorimeter above the threshold = 10 MeV. */
    TH1F* h_time_endcaps_Thr10MeV{nullptr};
    /** Histogram: Reconstructed signal time for the barrel calorimeter above the threshold = 50 MeV. */
    TH1F* h_time_barrel_Thr50MeV{nullptr};
    /** Histogram: Reconstructed signal time for the endcap calorimeter above the threshold = 50 MeV. */
    TH1F* h_time_endcaps_Thr50MeV{nullptr};
    /** Histogram: Fit quality flag (0 - good, 1 - large amplitude, 3 - bad chi2). */
    TH1F* h_quality{nullptr};
    /** Histogram: Number of hits in each event w/o software threshold.  */
    TH1F* h_ncev{nullptr};
    /** Histogram: Number of hits in each event above the treshold = 10 MeV.  */
    TH1F* h_ncev_Thr10MeV{nullptr};
    /** Histogram: Trigger tag flag #1. */
    TH1F* h_trigtag1{nullptr};
    /** Histogram: Flag of ADC samples. */
    TH1F* h_adc_flag{nullptr};
    /** Histogram: Fraction of ADC samples in event (w/o 8736 ADC samples). */
    TH1F* h_adc_hits{nullptr};
    /** Histogram: Trigger time vs. Trig Cell ID.  */
    TH2F* h_trigtime_trigid{nullptr};
    /** Histogram: Trigger tag flag #2 vs. Trig Cell ID.   */
    TH2F* h_trigtag2_trigid{nullptr};
    /** Histogram: Pedestal Average vs. Cell ID.   */
    TH2F* h_pedmean_cellid{nullptr};
    /** Histogram: Pedestal rms error vs. Cell ID.   */
    TH2F* h_pedrms_cellid{nullptr};
  };
}; // end Belle2 namespace
