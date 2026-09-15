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
#include <vector>

class TTree;

namespace Belle2 {

  class ECLWaveformDataUp;
  class ECLNoiseDataUp;
  class ECLWFAlgoParamsUp;
  class ECLHit;
  class ECLSimHit;
  class ECLDigit;
  class ECLDsp;
  class ECLDspWithExtraMCInfo;
  class ECLTrig;
  class ECLWaveforms;

  /** The upgraded ECL digitizer module.
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

     graph.module('ECLDigitizerUp', [ECLHit], [ECLDigit,ECLDsp])
     graph.relation(ECLDigitizerUp, ECLHit)
     graph.relation(ECLDigitizerUp, ECLDigit)
     graph.relation(ECLDigitizerUp, ECLDsp)
     graph.relation(ECLDigitizerUp, ECLDspWithExtraMCInfo)
     \endcorrelationdiagram

   */
  class ECLDigitizerUpModule : public Module {
  public:

    /** Constructor.
     */
    ECLDigitizerUpModule();

    /** Destructor.
     */
    ~ECLDigitizerUpModule();


    /** Initialize variables  */
    virtual void initialize() override;

    /** Nothing so far.*/
    virtual void beginRun() override;

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event() override;

    /** Nothing so far. */
    virtual void endRun() override;

    /** Free memory */
    virtual void terminate() override;

  private:
    using AlgoParams   = ECL::EclConfigurationUp::AlgoParams;   /**< algorithm parameters */
    using FitParams    = ECL::EclConfigurationUp::FitParams;    /**< fit parameters */
    using SignalSample = ECL::EclConfigurationUp::SignalSample; /**< signal sample */
    using AdcCounts    = ECL::EclConfigurationUp::AdcCounts;    /**< ADC counts */

    using IntArrayGridXAdc = FitParams::IntArrayGridXAdc; /**<  weighting coefficients for time and amplitude calculation */
    using IntArraySubXAdc = FitParams::IntArraySubXAdc;  /**<  weighting coefficients amplitude calculation. Time is fixed by trigger */
    using UIntPair = std::pair<unsigned int, unsigned int>; /**< a pair of unsigned ints */

    /** Indices in arrays with info on ECL channels */
    struct CrystalLinks {
      /** Index in EclAlgo tree */
      short unsigned int idn;
      /** Index in EclNoise tree */
      short unsigned int inoise;
      /** Index of associated (waveform parameters, fit parameters) pair */
      short unsigned int ifunc;
      /** Index in m_shapeSignalSamples */
      short unsigned int iss;
    };
    /** calibration constants per channel */
    struct Calibration {
      float ascale; /**< amplitude scale */
      float tshift; /**< time shift */
    };

    /** function wrapper for waveform fit */
    void shapeFitterWrapper(int j, const int* fitA, int triggerTime,
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
    void repack(const ECLWFAlgoParamsUp&, AlgoParams&);
    /** load waveform fit parameters for the shapeFitter function */
    void getFitParams(const ECLWaveformDataUp&, const ECLWFAlgoParamsUp&, FitParams&);
    /** Fill the waveform array with electronic noise and pedestal for channel j [0-8735]. */
    void makeElectronicNoiseAndPedestal(int j, int* fitA);

    /** Module parameters */
    std::string m_eclWaveformsName{""};

    double m_waveformThresholdOverride{-1.0};
    double m_dspWithExtraMCInfoThreshold{0.02};
    double m_unitscale{1.0};

    unsigned int m_compAlgo{2u};
    int m_adcThreshold{25};

    bool m_background{false};
    bool m_calibration{false};
    bool m_inter{true};
    bool m_noElectronicNoise{false};
    bool m_waveformMaker{false};
    bool m_storeDspWithExtraMCInfo{false};
    bool m_trigTime{false};
    bool m_hadronPulseShape{false};
    bool m_dspDataTest{false};
    bool m_useWaveformParameters{false};
    bool m_timeCalib{true};

    /** Always load waveform parameters at least once */
    bool m_loadOnce{true};

    /** Lookup table for ECL channels. */
    std::vector<CrystalLinks> m_tbl;

    /** Fit algorithm parameters shared by group of crystals */
    std::vector<AlgoParams> m_idn; /**< parameters that needs for waveform fit */
    std::vector<FitParams> m_fitparams; /**< Pairs of (waveform parameters, fit parameters) */
    std::vector<ECLNoiseDataUp> m_noise; /**< parameters for correlated noise simulation */
    std::vector<SignalSample> m_shapeSignalSamples; /**< tabulated shape line */
    std::vector<SignalSample> m_hadronShapeSignalSamples; /**< tabulated shape line for hadron shape simulations */

    /** Storage for adc hits from entire calorimeter (8736 crystals) */
    std::vector<AdcCounts> m_adc;  /**< ACD counts */

    /** Storage for calibration constants */
    std::vector<Calibration> m_calib;

    /** Storage for waveform saving thresholds*/
    std::vector<double> m_waveformThresholds;

    /** storage for trigger time in each ECL. The crate trigger time
     *  is an even number from 0 to 142, so here it is stored as
     *  numbers from 0 to 71 inclusive.
     */
    unsigned char m_ttime[ECL::ECL_CRATES] = {};

    /** Hadron signal shapes. */
    DBObjPtr<ECLDigitWaveformParametersForMC> m_waveformParametersMC{"ECLDigitWaveformParametersForMC"};

    /** CellID-specific signal shapes. */
    DBObjPtr<TTree> m_waveformParameters{"ECLWFParameters"};

    /** Shape fitting algorithm parameters. */
    DBObjPtr<TTree> m_algoParameters{"ECLWFAlgoParams"};

    /** Electronics noise covariance matrix. */
    DBObjPtr<TTree> m_noiseParameters{"ECLWFNoiseParams"};

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
