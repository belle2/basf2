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
#include <ecl/mapper/ECLChannelMapper.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/* C++ headers. */
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
    ~ECLDigitizerModule();


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
    using algoparams_t   = ECL::EclConfiguration::algoparams_t;   /**< algorithm parameters */
    using fitparams_t    = ECL::EclConfiguration::fitparams_t;    /**< fit parameters */
    using signalsample_t = ECL::EclConfiguration::signalsample_t; /**< signal sample */
    using adccounts_t    = ECL::EclConfiguration::adccounts_t;    /**< ADC counts */

    using int_array_192x16_t = fitparams_t::int_array_192x16_t; /**<  weighting coefficients for time and amplitude calculation */
    using int_array_24x16_t  =
      fitparams_t::int_array_24x16_t;  /**<  weighting coefficients amplitude calculation. Time is fixed by trigger */
    using uint_pair_t        = std::pair<unsigned int, unsigned int>; /**< a pair of unsigned ints */

    /** Indices in arrays with info on ECL channels */
    struct crystallinks_t {
      /** Index in EclAlgo tree */
      short unsigned int idn;
      /** Index in EclNoise tree */
      short unsigned int inoise;
      /** Index of associated (waveform parameters, fit parameters) pair */
      short unsigned int ifunc;
      /** Index in m_ss */
      short unsigned int iss;
    };

    /** Lookup table for ECL channels. */
    std::vector<crystallinks_t> m_tbl;

    /** Fit algorihtm parameters shared by group of crystals */
    std::vector<algoparams_t> m_idn; /**< parameters that needs for waveform fit */
    std::vector<fitparams_t> m_fitparams; /**< Pairs of (waveform parameters, fit parameters) */
    std::vector<ECLNoiseData> m_noise; /**< parameters for correlated noise simulation */
    std::vector<signalsample_t> m_ss; /**< tabulated shape line */
    std::vector<signalsample_t> m_ss_HadronShapeSimulations; /**< tabulated shape line for hadron shape simulations */

    /** Storage for adc hits from entire calorimeter (8736 crystals) */
    std::vector<adccounts_t> m_adc;  /**< ACD counts */

    /** calibration constants per channel */
    struct calibration_t {
      float ascale; /**< amplitude scale */
      float tshift; /**< time shift */
    };
    /** Storage for calibration constants */
    std::vector<calibration_t> m_calib;

    /** Storage for waveform saving thresholds*/
    std::vector<double> m_Awave;

    /** storage for trigger time in each ECL. The crate trigger time
     *  is an even number from 0 to 142, so here it is stored as
     *  numbers from 0 to 71 inclusive.
     */
    unsigned char m_ttime[ECL::ECL_CRATES] = {};

    /** function wrapper for waveform fit */
    void shapeFitterWrapper(const int j, const int* FitA, const int m_ttrig,
                            int& m_lar, int& m_ltr, int& m_lq, int& m_chi) const ;

    /** Always load waveform parameters at least once */
    bool m_loadOnce = true;

    /** callback hadron signal shapes from database*/
    void callbackHadronSignalShapes();

    /** read Shaper-DSP data from root file */
    void readDSPDB();
    /** Emulate response of energy deposition in a crystal and attached photodiode and make waveforms*/
    void shapeSignals();
    /** Produce and compress waveforms for beam background overlay */
    void makeWaveforms();
    /** repack waveform fit parameters from ROOT format to plain array of unsigned short for the shapeFitter function */
    void repack(const ECLWFAlgoParams&, algoparams_t&);
    /** load waveform fit parameters for the shapeFitter function */
    void getfitparams(const ECLWaveformData&, const ECLWFAlgoParams&, fitparams_t&);
    /** fill the waveform array FitA by electronic noise and bias it for channel J [0-8735]*/
    void makeElectronicNoiseAndPedestal(int j, int* FitA);

    /** Hadron signal shapes. */
    DBObjPtr<ECLDigitWaveformParametersForMC> m_waveformParametersMC;

    /** CellID-specific signal shapes. */
    DBObjPtr<TTree> m_waveformParameters;

    /** Shape fitting algorithm parameters. */
    DBObjPtr<TTree> m_algoParameters;

    /** Electronics noise covariance matrix. */
    DBObjPtr<TTree> m_noiseParameters;

    /** Crystal electronics. */
    DBObjPtr<ECLCrystalCalib> m_CrystalElectronics{"ECLCrystalElectronics"};

    /** Crystal energy. */
    DBObjPtr<ECLCrystalCalib> m_CrystalEnergy{"ECLCrystalEnergy"};

    /** Crystal electronics time. */
    DBObjPtr<ECLCrystalCalib> m_CrystalElectronicsTime{"ECLCrystalElectronicsTime"};

    /** Crystal time offset. */
    DBObjPtr<ECLCrystalCalib> m_CrystalTimeOffset{"ECLCrystalTimeOffset"};

    /** Crate time offset. */
    DBObjPtr<ECLCrystalCalib> m_CrateTimeOffset{"ECLCrateTimeOffset"};

    /** MC time offset. */
    DBObjPtr<ECLCrystalCalib> m_MCTimeOffset{"ECLMCTimeOffset"};

    /** FPGA waveform. */
    DBObjPtr<ECLCrystalCalib> m_FPGAWaveform{"ECL_FPGA_StoreWaveform"};

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

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

    /** Module parameters */
    bool m_background;  /**< background flag */
    bool m_calibration;  /**< calibration flag */
    bool m_inter; /**< internuclear counter effect */
    bool m_waveformMaker; /**< produce only waveform digits */
    bool m_storeDspWithExtraMCInfo;  /**< DSP with extra info flag */
    unsigned int m_compAlgo; /**< compression algorithm for background waveforms */
    int m_ADCThreshold; /**< ADC threshold for wavefom fits*/
    double m_WaveformThresholdOverride; /**< If gt 0, value will override ECL_FPGA_StoreWaveform and apply value (in GeV) as threshold for all crystals for waveform saving*/
    double m_DspWithExtraMCInfoThreshold;  /**< Energy threshold above which to store DSPs with extra information */
    bool m_trigTime; /**< Use trigger time from beam background overlay */
    std::string m_eclWaveformsName;   /**< name of background waveforms storage*/
    bool m_HadronPulseShape; /**< hadron pulse shape flag */

    bool m_dspDataTest; /**< DSP data usage flag */
    /** If true, use m_waveformParameters, m_algoParameters, m_noiseParameters.
     *  If false, use the data from ecl/data/ECL-WF.root or ECL-WF-BG.root
     */
    bool m_useWaveformParameters;
    /** Normalization coefficient for ECL signal shape.
     *  If positive, use same static value for all ECL channels.
     *  If negative, calculate it dynamically at beginRun().
     *  (for default shape parameters, the static value is 27.7221)
     */
    double m_unitscale;
  };
}//Belle2
