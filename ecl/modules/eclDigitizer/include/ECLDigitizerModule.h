/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLDigitWaveformParametersForMC.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/digitization/EclConfigurationUp.h>
#include <ecl/mapper/ECLChannelMapper.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/* C++ headers. */
#include <string>
#include <utility>
#include <variant>
#include <vector>

class TTree;

namespace Belle2 {

  class ECLWaveformData;
  class ECLNoiseData;
  class ECLWFAlgoParams;
  class ECLHit;
  class ECLSimHit;
  class ECLDigit;
  class ECLDsp;
  class ECLDspWithExtraMCInfo;
  class ECLTrig;
  class ECLWaveforms;

  /** The ECLDigitizer module.
   *
   * This module is responsible to digitize all hits found in the ECL from ECLHit
   * First, we simulate the sampling array by waveform and amplitude of hit, and
   * smear this sampling array by corresponding error matrix.
   * We then fit the array as hardware of shaper DSP board to obtain the fit result
   * of amplitude, time and quality.
   * The initial parameters of fit and algorithm are same as hardware.
   * This module outputs two array which are ECLDsp and ECLDigit.
   * An additional array with more MC information for ECLDsp studies is created upon user request.

     \correlationdiagram
     ECLHit = graph.data('ECLHit')
     ECLDigit   = graph.data('ECLDigit')
     ECLDsp = graph.data('ECLDsp')

     graph.module('ECLDigitizer', [ECLHit], [ECLDigit,ECLDsp])
     graph.relation(ECLDigitizer, ECLHit)
     graph.relation(ECLDigitizer, ECLDigit)
     graph.relation(ECLDigitizer, ECLDsp)
     graph.relation(ECLDigitizer, ECLDspWithExtraMCInfo)
     \endcorrelationdiagram

   */
  class ECLDigitizerModule : public Module {
  public:

    /** Constructor.
     */
    ECLDigitizerModule();

    /** Destructor.
     */
    ~ECLDigitizerModule() = default;

    /** Initialize variables  */
    void initialize() override;

    /** Nothing so far.*/
    void beginRun() override;

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    void event() override;

    /** Nothing so far. */
    void endRun() override;

    /** Free memory */
    void terminate() override;

  private:
    template <typename Config>
    /** Backend to select either current or upgraded ShaperDSP configuration */
    struct DigitizerBackend {
      using Configuration = Config; /**< ECL electronics configuration type. */
      using AlgoParams = typename Config::AlgoParams; /**< Algorithm parameters type. */
      using FitParams = typename Config::FitParams; /**< Fit parameters type. */
      using SignalSample = typename Config::SignalSample; /**< Signal sample type. */
      using AdcCounts = typename Config::AdcCounts; /**< ADC counts type. */

      std::vector<AlgoParams> idn; /**< Algorithm parameters shared by groups of crystals. */
      std::vector<FitParams> fitParams; /**< Waveform fit parameters. */
      std::vector<SignalSample> shapeSignalSamples; /**< Tabulated signal shapes. */
      std::vector<AdcCounts> adc; /**< ADC samples for all ECL crystals. */
    };

    template <typename BackendType>
    /** Implicit shape signal function depending on the selected backend */
    void shapeSignalsImpl(BackendType& backend);

    /** Current ECL electronics configuration */
    using CurrentBackend = DigitizerBackend<ECL::EclConfiguration>;
    /** Upgraded ECL electronics configuration */
    using UpgradeBackend = DigitizerBackend<ECL::EclConfigurationUp>;
    /** Backend to select either current or upgraded ShaperDSP configuration */
    using Backend = std::variant<CurrentBackend, UpgradeBackend>;

    using SignalSample = ECL::EclConfiguration::SignalSample; /**< signal sample */

    using UIntPair        = std::pair<unsigned int, unsigned int>; /**< a pair of unsigned ints */

    /** Indices in arrays with info on ECL channels */
    struct CrystalLinks {
      /** Index in EclAlgo tree */
      short unsigned int idn;
      /** Index in EclNoise tree */
      short unsigned int inoise;
      /** Index of associated (waveform parameters, fit parameters) pair */
      short unsigned int ifunc;
      /** Index in the sampled signal-shape pool */
      short unsigned int iss;
    };
    /** calibration constants per channel */
    struct Calibration {
      float ascale; /**< amplitude scale */
      float tshift; /**< time shift */
    };

    /** function wrapper for waveform fit */
    void shapeFitterWrapper(const int j, const int* fitA, const int triggerTime,
                            int& amplitude, int& time, int& quality, int& chi2) const ;
    /** callback hadron signal shapes from database*/
    void callbackHadronSignalShapes();
    /** read Shaper-DSP data from root file */
    void readDSPDB();
    /** Emulate response of energy deposition in a crystal and attached photodiode and make waveforms*/
    void shapeSignals();
    /** Produce and compress waveforms for beam background overlay */
    void makeWaveforms();
    /** repack waveform fit parameters from ROOT format to plain array of unsigned short for the shapeFitter function */
    template <typename AlgoParamsType>
    void repack(const ECLWFAlgoParams&, AlgoParamsType&);
    /** load waveform fit parameters for the shapeFitter function */
    template <typename Config>
    void getFitParams(const ECLWaveformData&, const ECLWFAlgoParams&, typename Config::FitParams&);
    /** fill the waveform array with electronic noise and pedestal for channel j [0-8735] */
    void makeElectronicNoiseAndPedestal(int j, int* fitA);

    /** Electronics-dependent digitization data. */
    Backend m_backend;

    /** Module parameters */

    /** name of background waveforms storage */
    std::string m_eclWaveformsName{""};

    /** Override for the waveform-saving threshold in GeV; if negative, use the DB value. */
    double m_waveformThresholdOverride{-1.0};
    /** Energy threshold above which DSPs with extra MC information are stored. */
    double m_dspWithExtraMCInfoThreshold{0.02};
    /** Normalization coefficient for the ECL signal shape. */
    double m_unitscale{-1.0};
    /** Compression algorithm for background waveforms; -1 selects the electronics default. */
    int m_compAlgo{-1};
    /** ADC threshold for waveform fits. */
    int m_adcThreshold{25};
    /** Background flag. */
    bool m_background{false};
    /** Calibration flag. */
    bool m_calibration{false};
    /** Enable the internuclear counter effect. */
    bool m_inter{true};
    /** Disable electronic noise when generating waveforms. */
    bool m_noElectronicNoise{false};
    /** Produce only waveform digits. */
    bool m_waveformMaker{false};
    /** Store DSP objects with additional MC information. */
    bool m_storeDspWithExtraMCInfo{false};
    /** Use trigger time from beam-background overlay. */
    bool m_trigTime{false};
    /** Enable hadron pulse shapes. */
    bool m_hadronPulseShape{true};
    /** DSP data usage flag. */
    bool m_dspDataTest{false};
    /** Enable upgraded electronics digitization */
    bool m_eclUpgrade{false};
    /** Always load waveform parameters at least once */
    bool m_loadOnce{true};

    /** Lookup table for ECL channels. */
    std::vector<CrystalLinks> m_tbl;

    /** Fit algorithm parameters shared by group of crystals */
    std::vector<ECLNoiseData> m_noise; /**< parameters for correlated noise simulation */
    std::vector<SignalSample> m_hadronShapeSignalSamples; /**< tabulated shape line for hadron shape simulations */

    /** Storage for calibration constants */
    std::vector<Calibration> m_calib;

    /** Storage for waveform saving thresholds*/
    std::vector<double> m_waveformThresholds;

    /** storage for trigger time in each ECL. The crate trigger time
     *  is an even number from 0 to c_nTriggerCounts, so here it is stored as
     *  numbers from 0 to c_nTriggerCounts/2 inclusive.
     */
    unsigned char m_ttime[ECL::ECL_CRATES] = {};

    /** Hadron signal shapes. */
    DBObjPtr<ECLDigitWaveformParametersForMC> m_waveformParametersMC{"ECLDigitWaveformParametersForMC"};

    /** CellID-specific signal shapes for current electronics. */
    DBObjPtr<TTree> m_waveformParameters{"ECLWFParameters"};

    /** Shape fitting algorithm parameters for current electronics. */
    DBObjPtr<TTree> m_algoParameters{"ECLWFAlgoParams"};

    /** Electronics noise covariance matrix for current electronics. */
    DBObjPtr<TTree> m_noiseParameters{"ECLWFNoiseParams"};

    /** CellID-specific signal shapes for upgraded electronics. */
    DBObjPtr<TTree> m_waveformParametersUp{"ECLWFParametersUp", false};

    /** Shape fitting algorithm parameters for upgraded electronics. */
    DBObjPtr<TTree> m_algoParametersUp{"ECLWFAlgoParamsUp", false};

    /** Electronics noise covariance matrix for upgraded electronics. */
    DBObjPtr<TTree> m_noiseParametersUp{"ECLWFNoiseParamsUp", false};

    /** Crystal electronics. */
    DBObjPtr<ECLCrystalCalib> m_crystalElectronics{"ECLCrystalElectronics"};

    /** Crystal energy. */
    DBObjPtr<ECLCrystalCalib> m_crystalEnergy{"ECLCrystalEnergy"};

    /** Crystal electronics time. */
    DBObjPtr<ECLCrystalCalib> m_crystalElectronicsTime{"ECLCrystalElectronicsTime"};

    /** Crystal time offset. */
    DBObjPtr<ECLCrystalCalib> m_crystalTimeOffset{"ECLCrystalTimeOffset"};

    /** Crate time offset. */
    DBObjPtr<ECLCrystalCalib> m_crateTimeOffset{"ECLCrateTimeOffset"};

    /** MC time offset. */
    DBObjPtr<ECLCrystalCalib> m_mcTimeOffset{"ECLMCTimeOffset"};

    /** FPGA waveform. */
    DBObjPtr<ECLCrystalCalib> m_fpgaWaveform{"ECL_FPGA_StoreWaveform"};

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /* Input arrays. */

    /** Hits array. */
    StoreArray<ECLHit> m_eclHits;

    /** Diode hits array. */
    StoreArray<ECLHit> m_eclDiodeHits;

    /** SimHits array. */
    StoreArray<ECLSimHit> m_eclSimHits;

    /** Compressed waveforms. */
    StoreObjPtr<ECLWaveforms> m_eclWaveforms;

    /** Output Arrays */

    /** Waveform fit result. */
    StoreArray<ECLDigit> m_eclDigits;

    /** Generated waveforms. */
    StoreArray<ECLDsp> m_eclDsps;

    /** Generated waveforms with extra MC information. */
    StoreArray<ECLDspWithExtraMCInfo> m_eclDspsWithExtraMCInfo;

    /** Trigger information. */
    StoreArray<ECLTrig> m_eclTrigs;

    /** Channel Mapper */
    ECL::ECLChannelMapper m_eclMapper; /**< channel mapper to utilize trigger information */

  };
}//Belle2
