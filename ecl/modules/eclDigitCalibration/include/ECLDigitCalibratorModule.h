/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITCALIBRATORMODULE_H_
#define ECLDIGITCALIBRATORMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>

// ECL
#include <framework/database/DBArray.h>
#include <ecl/dbobjects/ECLCalibrationDigit.h>

// OTHER
#include <vector>

// ROOT
#include <TRandom3.h>
#include <TMatrixFSym.h>

namespace Belle2 {
  namespace ECL {
    class ECLDigitCalibratorModule : public Module {

    public:

      /** Constructor.
       */
      ECLDigitCalibratorModule();

      /** Destructor.
       */
      ~ECLDigitCalibratorModule();

      /** Initialize variables. */
      virtual void initialize();

      /** begin run.*/
      virtual void beginRun();

      /** event per event.
       */
      virtual void event();

      /** end run. */
      virtual void endRun();

      /** terminate.*/
      virtual void terminate();

    protected:

    private:

      const int c_nCrystals = 8736;
      std::vector < float > m_calibrationAmplitudesLowEnergy;  //< vector with single crystal calibration amplitudes low energy
      std::vector < float > m_calibrationEnergiesLowEnergy;  //< vector with single crystal calibration energy values low energy
      std::vector < float > m_calibrationAmplitudesHighEnergy;  //< vector with single crystal calibration amplitudes high energy
      std::vector < float > m_calibrationEnergiesHighEnergy;  //< vector with single crystal calibration energy values high energy
      std::vector < double > m_calibrationC0;  //< vector with single crystal calibration energy values c0
      std::vector < double > m_calibrationC1;  //< vector with single crystal calibration  values c1
      DBArray<ECLCalibrationDigit> m_calibrationLow;  //< single crystal calibration constants low energy
      DBArray<ECLCalibrationDigit> m_calibrationHigh;  //< single crystal calibration constants high energy

      double getCalibratedEnergy(int cellid, int energy); //< log interpolated value from the calibration amplitudes and energies
      double getCalibratedTime(int cellid, int time); //< timing correction
      void prepareCalibrationConstants(); //< reads calibration constants, performs checks, put them into a vector

    };
  } // end ECL namespace
} // end Belle2 namespace

#endif
