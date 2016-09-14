/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Digit Calibration.                                                     *
 *                                                                        *
 * This  module converts the fitted amplitude into calibrated energy,     *
 * fitted time into calibrated time, and determines the time resolution   *
 * per digit. It furthermore determines the background level by counting  *
 * out of time digits above a certain energy threshold.                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Chris Hearty (hearty@physics.ubc.ca) (CH)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITCALIBRATORMODULE_H_
#define ECLDIGITCALIBRATORMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

// ECL
#include <framework/database/DBArray.h>
#include <ecl/dbobjects/ECLDigitEnergyConstants.h>
#include <ecl/dbobjects/ECLDigitTimeConstants.h>

// OTHER
#include <vector>

// ROOT
#include <TRandom3.h>
#include <TMatrixFSym.h>

namespace Belle2 {
  namespace ECL {

    /** Class to find calibrate digits and convert waveform fit information to physics quantities */
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

      /** Name of the ECLDigit.*/
      virtual const char* eclDigitArrayName() const
      { return "ECLDigits" ; }

      /** Name of the ECLCalDigit.*/
      virtual const char* eclCalDigitArrayName() const
      { return "ECLCalDigits" ; }

      /** Name of the ECLEventInformation.*/
      virtual const char* eclEventInformationName() const
      { return "ECLEventInformation" ; }

    protected:

    private:

      double m_backgroundEnergyCut;  /**< Energy cut for background level counting. */
      double m_backgroundTimingCut;  /**< Timing window for background level counting. */

      const int c_nCrystals = 8736;  /**< Number of ECL crystals. */
      std::vector < double > m_calibrationEnergyHighRatio;  /**< vector with single crystal calibration ratios (high energy) */
      std::vector < double > m_calibrationTimeOffset;  /**< vector with time calibration constant offsets */

      DBArray<ECLDigitEnergyConstants> m_calibrationEnergyHigh;  /**< single crystal calibration constants high energy */
      DBArray<ECLDigitTimeConstants> m_calibrationTime;  /**< single crystal calibration constants time */

      double getCalibratedEnergy(const int cellid, const int energy) const; /**< energy calibration */
      double getCalibratedTime(const int cellid, const int time, const bool fitfailed) const; /**< timing correction. */
      double getCalibratedTimeResolution(const int cellid, const double energy, const bool fitfailed) const; /**< timing resolution. */
      double getInterpolatedTimeResolution(const double x, const int bin) const; /**< timing resolution interpolation. */
      void prepareEnergyCalibrationConstants(); /**< reads calibration constants, performs checks, put them into a vector */
      void prepareTimeCalibrationConstants(); /**< reads calibration constants, performs checks, put them into a vector */
      void determineBackgroundECL(); /**< count out of time digits to determine baclground levels */

      double m_timeInverseSlope; /**< Time calibration inverse slope "a". */
      double m_timeResolutionPointResolution[4]; /**< Time resolution calibration interpolation parameter "Resolution". */
      double m_timeResolutionPointX[4];  /**< Time resolution calibration interpolation parameter "x = 1/E (GeV)". */
      const double c_timeResolutionForFitFailed  = 1.0e9; /**< Time resolution for failed fits". */
      const double c_timeResolutionForZeroEnergy =
        1.0e9; /**< Time resolution for (very close to) zero energy digits (should not happen)". */
      const double c_timeForFitFailed            = 0.0; /**< Time for failed fits". */
      const int c_MinimumAmplitude               = 1; /**< Minimum amplitude". */
      const double c_energyForSmallAmplitude     = 0.0; /**< Energy for small amplitudes". */

    };

    /** Class derived from ECLDigitCalibratorModule, only difference are the names */
    class ECLDigitCalibratorPureCsIModule : public ECLDigitCalibratorModule {
    public:
      /** PureCsI Name of the ECLDigitsPureCsI.*/
      virtual const char* eclDigitArrayName() const override
      { return "ECLDigitsPureCsI" ; }

      /** PureCsI Name of the ECLCalDigitsPureCsI.*/
      virtual const char* eclCalDigitArrayName() const override
      { return "ECLCalDigitsPureCsI" ; }

      /** PureCsI Name of the ECLEventInformationPureCsI.*/
      virtual const char* eclEventInformationName() const override
      { return "ECLEventInformationPureCsI" ; }


    };

  } // end ECL namespace
} // end Belle2 namespace

#endif
