/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITIZERPURECSIMODULE_H_
#define ECLDIGITIZERPURECSIMODULE_H_

#include <framework/core/Module.h>
#include <ecl/digitization/EclConfigurationPure.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <vector>

class TH1F;

namespace Belle2 {
  /** The ECLDigitizerPureCsI module.
   *
   * This module is responsible to digitize all hits found in the ECL from ECLHit
   * First, we simualte the sampling array by waveform and amplitude of hit, and
   * smear this sampling array by corresponding error matrix.
   * We then fit the array as hardware of shaper DSP board to obtain the fit result
   * of amplitude, time and quality.
   * The initial parameters of fit and algorithm are same as hardware.
   * This module outputs two array which are ECLDsp and ECLDigit.

   */
  class ECLDigitizerPureCsIModule : public Module {
  public:

    /** Constructor.
     */
    ECLDigitizerPureCsIModule();

    /** Destructor.
     */
    ~ECLDigitizerPureCsIModule();


    /** Initialize variables  */
    virtual void initialize();

    /** Nothing so far.*/
    virtual void beginRun();

    /** Actual digitization of all pure CsI hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event();

    /** Nothing so far. */
    virtual void endRun();

    /** Free memory. */
    virtual void terminate();

  private:
    /** ECL ring ID. */
    int m_thetaID[ECL::EclConfigurationPure::m_nch];
    /** Returns ring ID for a certain crystal. */
    void mapGeometry();
    /** Returns 1 if corresponding crystal is set as pure CsI crystal. */
    bool isPureCsI(int cellId)
    {
      if (cellId > ECL::EclConfigurationPure::m_nch) return false;
      int thId = m_thetaID[cellId - 1];
      if (thId >= m_thetaIdMin && thId <= m_thetaIdMax) return true;
      return false;
    }

    /** Offsets for storage of ECL channels (that's actually from an older comment, not quite sure this is correct). */
    struct crystallinks_t {
      short unsigned int idn;
      short unsigned int inoise;
      short unsigned int ifunc;
      short unsigned int iss;
    };

    /** Lookup table for ECL channels. */
    std::vector<crystallinks_t> m_tbl;

    /** Type of fitting parameters. */
    using fitparams_type = ECL::EclConfigurationPure::fitparamspure_t;
    /** Type of signal sample. */
    using signalsample_type = ECL::EclConfigurationPure::signalsamplepure_t;
    /** Type of ADC counts. */
    using adccounts_type = ECL::EclConfigurationPure::adccountspure_t;

    /** Fitting parameters. */
    std::vector<fitparams_type> m_fitparams;
    /** Tabulated shape line. */
    std::vector<signalsample_type> m_ss;

    /** Storage for adc hits from entire calorimeter (8736 crystals). */
    std::vector<adccounts_type> m_adc;
    /** Parameters for correlated noise stimation. */
    std::vector<ECLNoiseData> m_noise;
    /** read Shaper-DSP data from root file. */
    void readDSPDB();

    /** Event number */
    int    m_nEvent;

    /** Module parameters. */
    /** Ring ID of first pure CsI ring. */
    int m_thetaIdMin;
    /** Ring ID of last pure CsI ring in FWD. */
    int m_thetaIdMax;
    /** Flag to set covariance matrix for WF with beam-bkg. */
    bool m_background;
    /** Flag to use the DigitizerPureCsI for Waveform fit Covariance Matrix calibration. */
    bool m_calibration;
    /** Flag to use a diagonal (neutral) Covariance matrix. */
    bool m_NoCovMatrix;
    /** multiplication factor to get adc tick from trigger tick. */
    int m_tickFactor;
    /** Trigger resolution. */
    double m_sigmaTrigger;
    /** Electronic Noise energy equivalente in MeV. */
    double m_elecNoise;
    /** Resolution for a 1 MeV energy deposit. */
    double m_photostatresolution;
    /** Flag for debug mode. */
    bool m_debug;
    /** Fixed trigger time for testing purposes. */
    int m_testtrg;
    /** Shift in signal arrival time, for testing purposes. */
    double m_testsig;
    /** Fixed energy deposition in all crystals, for testing purposes. */
    double m_testenedep;
    /** Pure CsI digit array name. */
    static constexpr const char* eclDigitArrayName() { return "ECLDigitsPureCsI"; }
    /** Pure CsI DSP array name. */
    static constexpr const char* eclDspArrayName() { return "ECLDspsPureCsI"; }
    /** Pure CsI Info array name. */
    static constexpr const char* eclPureCsIInfoArrayName() { return "ECLPureCsIInfo"; }
  };
}//Belle2

#endif /* ECLDIGITIZERPURECSIMODULE_H_ */
