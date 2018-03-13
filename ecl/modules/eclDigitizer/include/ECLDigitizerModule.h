/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alex Bobrov                                              *
 *               Alexei Sibidanov                                         *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITIZERMODULE_H_
#define ECLDIGITIZERMODULE_H_

#include <framework/core/Module.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLWaveforms.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <vector>


namespace Belle2 {
  /** The ECLDigitizer module.
   *
   * This module is responsible to digitize all hits found in the ECL from ECLHit
   * First, we simualte the sampling array by waveform and amplitude of hit, and
   * smear this sampling array by corresponding error matrix.
   * We then fit the array as hardware of shaper DSP board to obtain the fit result
   * of amplitude, time and quality.
   * The initial parameters of fit and algorithm are same as hardware.
   * This module outputs two array which are ECLDsp and ECLDigit.

     \correlationdiagram
     ECLHit = graph.data('ECLHit')
     ECLDigit   = graph.data('ECLDigit')
     ECLDsp = graph.data('ECLDsp')

     graph.module('ECLDigitizer', [ECLHit], [ECLDigit,ECLDsp])
     graph.relation(ECLDigitizer, ECLHit)
     graph.relation(ECLDigitizer, ECLDigit)
     graph.relation(ECLDigitizer, ECLDsp)
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
    virtual void initialize();

    /** Nothing so far.*/
    virtual void beginRun();

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event();

    /** Nothing so far. */
    virtual void endRun();

    /** Free memory */
    virtual void terminate();

  private:
    using algoparams_t   = ECL::EclConfiguration::algoparams_t;
    using fitparams_t    = ECL::EclConfiguration::fitparams_t;
    using signalsample_t = ECL::EclConfiguration::signalsample_t;
    using adccounts_t    = ECL::EclConfiguration::adccounts_t;

    using int_array_192x16_t = fitparams_t::int_array_192x16_t; /**<  weighting coefficients for time and amplitude calculation */
    using int_array_24x16_t  =
      fitparams_t::int_array_24x16_t;  /**<  weighting coefficients amplitude calculation. Time is fixed by trigger */
    using uint_pair_t        = std::pair<unsigned int, unsigned int>;

    /** ffsets for storages of ECL channels */
    struct crystallinks_t {
      short unsigned int idn;
      short unsigned int inoise;
      short unsigned int ifunc;
      short unsigned int iss;
    };

    std::vector<crystallinks_t> m_tbl;

    /** Fit algorihtm parameters shared by group of crystals */
    std::vector<algoparams_t> m_idn; /**< parameters that needs for waveform fit */
    std::vector<fitparams_t> m_fitparams; /**<  */
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

    /** function wrapper for waveform fit */
    void shapeFitterWrapper(const int j, const int* FitA, const int m_ttrig,
                            int& m_lar, int& m_ltr, int& m_lq, int& m_chi) const ;

    /** load hadron signal shapes from database*/
    void loadHadronSignalShapes();
    /** read Shaper-DSP data from root file */
    void readDSPDB();
    /** Emulate response of energy deposition in a crystal and attached photodiode and make waveforms*/
    int shapeSignals();
    /** Produce and compress waveforms for beam background overlay */
    void makeWaveforms();
    /** repack waveform fit parameters from ROOT format to plain array of unsigned short for the shapeFitter function */
    void repack(const ECLWFAlgoParams&, algoparams_t&);
    /** load waveform fit parameters for the shapeFitter function */
    void getfitparams(const ECLWaveformData&, const ECLWFAlgoParams&, fitparams_t&);
    /** fill the waveform array FitA by electronic noise and bias it for channel J [0-8735]*/
    void makeElectronicNoiseAndPedestal(int j, int* FitA);

    /** input arrays */
    StoreArray<ECLHit>    m_eclHits;  /**< hits array  */
    StoreArray<ECLHit>    m_eclDiodeHits; /**< diode hits array  */
    StoreArray<ECLSimHit> m_eclSimHits; /**< SimHits array  */
    StoreObjPtr<ECLWaveforms> m_eclWaveforms; /**< compressed waveforms  */
    bool m_HadronPulseShape; /**< hadron pulse shape flag */
    /** Output Arrays */
    StoreArray<ECLDigit>  m_eclDigits;/**<  waveform fit result */
    StoreArray<ECLDsp>    m_eclDsps;/**<  generated waveforms */
    StoreArray<ECLTrig>   m_eclTrigs;/**< trigger information */

    /** Module parameters */
    bool m_background;  /**< background flag */
    bool m_calibration;  /**< calibration flag */
    bool m_inter; /**< internuclear counter effect */
    bool m_waveformMaker; /**< produce only waveform digits */
    unsigned int m_compAlgo; /**< compression algorithm for background waveforms */
    std::string m_eclWaveformsName;   /**< name of background waveforms storage*/
  };
}//Belle2

#endif /* ECLDIGITIZERMODULE_H_ */
