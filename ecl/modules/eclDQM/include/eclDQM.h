/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECL Data Quality Monitor (First Module)                                *
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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/DBObjPtr.h>

//ECL
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

class TH1F;
class TH2F;
class TProfile;

namespace Belle2 {

  class EventMetaData;
  class ECLDigit;
  class ECLDsp;
  class ECLTrig;
  class ECLCalDigit;
  class TRGSummary;

  /**
   * This module is created to monitor ECL Data Quality.
   */
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
    /** Geometry */
    ECL::ECLGeometryPar* m_geom{nullptr};
    /** StoreObjPtr EventMetaData */
    StoreObjPtr<EventMetaData> m_eventmetadata;
    /** StoreObjPtr TRGSummary  */
    StoreObjPtr<TRGSummary> m_l1Trigger;
    /** ECL channel mapper */
    ECL::ECLChannelMapper mapper;
    /** StoreArray ECLDigit */
    StoreArray<ECLDigit> m_ECLDigits;
    /** StoreArray ECLDsp */
    StoreArray<ECLDsp> m_ECLDsps;
    /** StoreArray ECLTrig */
    StoreArray<ECLTrig> m_ECLTrigs;
    /** StoreArray ECLCalDigit */
    StoreArray<ECLCalDigit> m_ECLCalDigits;
    /** PSD waveform amplitude threshold. */
    DBObjPtr<ECLCrystalCalib> m_calibrationThrApsd;

    /** Global event number. */
    int m_iEvent{ -1};
    /** Histogram directory in ROOT file. */
    std::string m_histogramDirectoryName;
    /** Upper threshold of energy deposition in event, [GeV]. */
    double m_EnergyUpperThr;
    /** Flag to select events triggered by delayed bhabha. */
    bool m_DPHYTTYP{0};

    /** Parameters for hit occ. histograms. */
    std::vector<double> m_HitThresholds = {};
    /** Parameters for histograms w/ total energy. */
    std::vector<double> m_TotalEnergyThresholds = {};
    /** Parameters for timing histograms. */
    std::vector<double> m_TimingThresholds = {};
    /** Parameters for number of hits histograms. */
    std::vector<double> m_HitNumberUpperLimits = {};
    /** Parameters for waveform histograms. */
    std::vector<std::string> m_WaveformOption;
    /** Container for energy. */
    std::vector<double> ecltot = {};
    /** Container for channel multiplicity. */
    std::vector<double> nhits = {};
    /** Vector to store psd wf amplitude threshold. */
    std::vector<int> v_totalthrApsd = {};


    /** WF sampling points for digit array.   */
    int m_DspArray[8736][31] = {};
    /** Pedestal average values.   */
    double m_PedestalMean[8736] = {};
    /** Pedestal rms error values.    */
    double m_PedestalRms[8736] = {};

    /** Histogram: Total event no (auxiliary) to normalize hit map . */
    TH1F* h_evtot{nullptr};
    /** Histogram: Event no for logic (auxiliary) to normalize logic waveform flow. */
    TH1F* h_evtot_logic{nullptr};
    /** Histogram: Event no for rand (auxiliary) to normalize rand waveform flow. */
    TH1F* h_evtot_rand{nullptr};
    /** Histogram: Event no for dphy (auxiliary) to normalize dphy waveform flow. */
    TH1F* h_evtot_dphy{nullptr};
    /** Histogram: Fit quality flag (0 - good, 1 - large amplitude, 3 - bad chi2). */
    TH1F* h_quality{nullptr};
    /** Histogram: Fit quality flag for waveform type 'other'. */
    TH1F* h_quality_other{nullptr};
    /** Histogram: Cell IDs w/ bad fit quality flag. */
    TH1F* h_bad_quality{nullptr};
    /** Histogram: Trigger tag flag #1. */
    TH1F* h_trigtag1{nullptr};
    /** Histogram: Fraction of digits above ADC threshold. */
    TH1F* h_adc_hits{nullptr};

    /** Histogram vector: Hit map. */
    std::vector<TH1F*> h_cids = {};
    /** Histogram vector: Total energy. */
    std::vector<TH1F*> h_edeps = {};
    /** Histogram vector: Reconstructed time for barrel. */
    std::vector<TH1F*> h_time_barrels = {};
    /** Histogram vector: Reconstructed time for endcaps. */
    std::vector<TH1F*> h_time_endcaps = {};
    /** Histogram vector: Channel multiplicity. */
    std::vector<TH1F*> h_ncevs = {};
    /** Histogram vector: Waveforms vs CellID. */
    std::vector<TH1F*> h_cells = {};
    /** Histogram: Normalize to psd hits for CellID. */
    TH1F* h_cell_psd_norm{nullptr};
    /** Histogram vector: Reconstructed signal time for all ECL crates above the threshold = 1 GeV. */
    std::vector<TH1F*> h_time_crate_Thr1GeV = {};
    /** Histogram: Trigger time vs. Trig Cell ID.  */
    TH2F* h_trigtime_trigid{nullptr};
    /** Histogram: Trigger tag flag #2 vs. Trig Cell ID.   */
    TH2F* h_trigtag2_trigid{nullptr};
    /** Histogram: Pedestal Average vs. Cell ID.   */
    TProfile* h_pedmean_cellid{nullptr};
    /** Histogram: Pedestal rms error vs. Cell ID.   */
    TProfile* h_pedrms_cellid{nullptr};
    /** Histogram: Pedestal rms error vs. Theta ID.   */
    TProfile* h_pedrms_thetaid{nullptr};
  };
}; // end Belle2 namespace
