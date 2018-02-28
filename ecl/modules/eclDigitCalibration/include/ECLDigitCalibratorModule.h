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

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>

// ECL
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dataobjects/ECLPureCsIInfo.h>

// OTHER
#include <vector>

// ROOT
#include <TRandom3.h>
#include <TMatrixFSym.h>
#include "TH1D.h"
#include "TFile.h"

namespace Belle2 {

  class EventLevelClusteringInfo;

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

    /** Name of the ECL pure CsI Information.*/
    virtual const char* eclPureCsIInfoArrayName() const
    { return "ECLPureCsIInfo" ; }

  protected:

  private:

    double m_backgroundEnergyCut;  /**< Energy cut for background level counting. */
    double m_backgroundTimingCut;  /**< Timing window for background level counting. */

    const int c_nCrystals = 8736;  /**< Number of ECL crystals. */

    std::vector < float > v_calibrationCrystalElectronics;  /**< single crystal electronics calibration as vector*/
    std::vector < float > v_calibrationCrystalElectronicsUnc;  /**< single crystal electronics calibration as vector uncertainty*/
    DBObjPtr<ECLCrystalCalib> m_calibrationCrystalElectronics;  /**< single crystal electronics calibration */

    std::vector < float > v_calibrationCrystalEnergy;  /**< single crystal energy calibration as vector*/
    std::vector < float > v_calibrationCrystalEnergyUnc;  /**< single crystal energy calibration as vector uncertainty*/
    DBObjPtr<ECLCrystalCalib> m_calibrationCrystalEnergy;  /**< single crystal energy calibration */

    std::vector < float > v_calibrationCrystalElectronicsTime;  /**< single crystal time calibration offset electronics as vector*/
    std::vector < float >
    v_calibrationCrystalElectronicsTimeUnc;  /**< single crystal time calibration offset electronics as vector uncertainty*/
    DBObjPtr<ECLCrystalCalib> m_calibrationCrystalElectronicsTime;  /**< single crystal time calibration offset electronics*/

    std::vector < float > v_calibrationCrystalTimeOffset;  /**< single crystal time calibration offset as vector*/
    std::vector < float > v_calibrationCrystalTimeOffsetUnc;  /**< single crystal time calibration offset as vector uncertainty*/
    DBObjPtr<ECLCrystalCalib> m_calibrationCrystalTimeOffset;  /**< single crystal time calibration offset*/

    std::vector < float > v_calibrationCrystalFlightTime;  /**< single crystal time calibration TOF as vector*/
    std::vector < float > v_calibrationCrystalFlightTimeUnc;  /**< single crystal time calibration TOF as vector uncertainty*/
    DBObjPtr<ECLCrystalCalib> m_calibrationCrystalFlightTime;  /**< single crystal time calibration TOF*/

    StoreObjPtr <EventLevelClusteringInfo> m_eventLevelClusteringInfo; /** event level clustering info */

    double m_timeInverseSlope; /**< Time calibration inverse slope "a". */

    double m_pureCsIEnergyCalib = 0.00005; /**< conversion factor from ADC counts to GeV. */
    double m_pureCsITimeCalib = 10.; /**< conversion factor from eclPureCsIDigitizer to ns. */
    double m_pureCsITimeOffset = 0.31; /**< ad-hoc offset correction for pureCsI timing/ */

    void initializeCalibration(); /**< reads calibration constants, performs checks, put them into a vector */
    void callbackCalibration(DBObjPtr<ECLCrystalCalib>& cal, std::vector<float>& constants,
                             std::vector<float>& constantsUnc); /**< reads calibration constants */



//      double getCalibratedEnergy(const int cellid, const int energy) const; /**< energy calibration */
//      double getCalibratedTime(const int cellid, const int time, const bool fitfailed) const; /**< timing correction. */
    double getT99(const int cellid, const double energy, const bool fitfailed, const int bgcount) const; /**< t99%. */
//      double getInterpolatedTimeResolution(const double x, const int bin) const; /**< timing resolution interpolation. */
//      void prepareEnergyCalibrationConstants(); /**< reads calibration constants, performs checks, put them into a vector */
//      void prepareTimeCalibrationConstants(); /**< reads calibration constants, performs checks, put them into a vector */
    int determineBackgroundECL(); /**< count out of time digits to determine baclground levels */

    double m_timeResolutionPointResolution[4]; /**< Time resolution calibration interpolation parameter "Resolution". */
    double m_timeResolutionPointX[4];  /**< Time resolution calibration interpolation parameter "x = 1/E (GeV)". */
    const double c_timeResolutionForFitFailed  = 1.0e9; /**< Time resolution for failed fits". */
    const double c_timeForFitFailed            = 0.0; /**< Time for failed fits". */
    const int c_MinimumAmplitude               = 1; /**< Minimum amplitude". */
    const double c_energyForSmallAmplitude     = 0.0; /**< Energy for small amplitudes". */

    // new time calibration from Kim and Chris
    std::string m_fileBackgroundName; /**< Background filename. */
    TFile* m_fileBackground; /**< Background file. */
    TH1D* m_th1dBackground; /**< Background histogram. */

    const double c_pol2Var1 = 1684.0; /**< 2-order fit for p1 Var1 + Var2*bg + Var3*bg^2. */
    const double c_pol2Var2 = 3080.0; /**< 2-order fit for p1. */
    const double c_pol2Var3 = -613.9; /**< 2-order fit for p1. */
    double m_pol2Max; /** < Maximum of p1 2-order fit to limit values */
    const int c_nominalBG = 183; /**< Number of out of time digits at BGx1.0. */
    double m_averageBG; /** < Average dose per crystal calculated from m_th1dBackground */
    const double c_minT99 = 3.5;

    bool m_simulatePure = 0; /** < Flag to set pure CsI simulation option */
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

} // end Belle2 namespace

