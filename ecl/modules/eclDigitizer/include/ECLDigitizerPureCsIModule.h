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
  namespace ECL {
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
      int m_thetaID[EclConfigurationPure::m_nch];
      void mapGeometry();
      bool isPureCsI(int cellId)
      {
        if (cellId > EclConfigurationPure::m_nch) return false;
        int thId = m_thetaID[cellId - 1];
        if (thId >= m_thetaIdMin && thId <= m_thetaIdMax) return true;
        return false;
      }

      struct crystallinks_t { // offsets for storages of ECL channels
        short unsigned int idn;
        short unsigned int inoise;
        short unsigned int ifunc;
        short unsigned int iss;
      };

      std::vector<crystallinks_t> m_tbl;

      using fitparams_type = EclConfigurationPure::fitparamspure_t;
      using signalsample_type = EclConfigurationPure::signalsamplepure_t;
      using adccounts_type = EclConfigurationPure::adccountspure_t;

      std::vector<fitparams_type> m_fitparams;
      std::vector<signalsample_type> m_ss;

      // Storage for adc hits from entire calorimeter (8736 crystals)
      std::vector<adccounts_type> m_adc;
      std::vector<ECLNoiseData> m_noise;
      /** read Shaper-DSP data from root file */
      void readDSPDB();

      /** Event number */
      int    m_nEvent;

      /** Module parameters */
      int m_thetaIdMin, m_thetaIdMax;
      bool m_background;
      bool m_calibration;
      int m_tickFactor;
      double m_sigmaTrigger;
      double m_elecNoise;
      double m_photostatresolution;
      bool m_debug;
      int m_testtrg;
      double m_testsig;
      static constexpr const char* eclDigitArrayName() { return "ECLDigitsPureCsI"; }
      static constexpr const char* eclDspArrayName() { return "ECLDspsPureCsI"; }
    };
  }//ECL
}//Belle2

#endif /* ECLDIGITIZERPURECSIMODULE_H_ */
