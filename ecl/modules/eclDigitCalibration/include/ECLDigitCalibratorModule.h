/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/utility/ECLTimingUtilities.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/* C++ headers. */
#include <vector>

class TH1F;
class TFile;

namespace Belle2 {

  class EventLevelClusteringInfo;

  class ECLCrystalCalib;
  class ECLDigit;
  class ECLDsp;
  class ECLCalDigit;
  class ECLPureCsIInfo;

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
    virtual void initialize() override;

    /** begin run.*/
    virtual void beginRun() override;

    /** event per event.
     */
    virtual void event() override;

    /** end run. */
    virtual void endRun() override;

    /** terminate.*/
    virtual void terminate() override;

    /** Name of the ECLDigit.*/
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    /** Name of the ECLDsp.*/
    virtual const char* eclDspArrayName() const
    { return "ECLDsps" ; }

    /** Name of the ECLCalDigit.*/
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Name of the EventLevelClusteringInfo.*/
    virtual const char* eventLevelClusteringInfoName() const
    { return "EventLevelClusteringInfo" ; }

    /** Name of the ECL pure CsI Information.*/
    virtual const char* eclPureCsIInfoArrayName() const
    { return "ECLPureCsIInfo" ; }

  protected:

  private:

    double m_backgroundEnergyCut;  /**< Energy cut for background level counting. */
    double m_backgroundTimingCut;  /**< Timing window for background level counting. */

    const int c_nCrystals = ECLElementNumbers::c_NCrystals;  /**< Number of ECL crystals. */

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

    std::vector < float > v_calibrationCrateTimeOffset;  /**< single crate time calibration offset as vector (per crystal) */
    std::vector < float > v_calibrationCrateTimeOffsetUnc;  /**< single crate time calibration offset as
                                                                 vector uncertainty (per crystal) */
    DBObjPtr<ECLCrystalCalib> m_calibrationCrateTimeOffset;  /**< single crate time calibration offset (per crystal) */

    std::vector < float > v_calibrationCrystalFlightTime;  /**< single crystal time calibration TOF as vector*/
    std::vector < float > v_calibrationCrystalFlightTimeUnc;  /**< single crystal time calibration TOF as vector uncertainty*/
    DBObjPtr<ECLCrystalCalib> m_calibrationCrystalFlightTime;  /**< single crystal time calibration TOF*/

    StoreObjPtr <EventLevelClusteringInfo> m_eventLevelClusteringInfo; /**< event level clustering info */

    StoreArray<ECLDigit> m_eclDigits; /**< storearray ECLDigit */
    StoreArray<ECLCalDigit> m_eclCalDigits; /**< storearray ECLCalDigit */
    StoreArray<ECLDsp> m_eclDsps; /**< storearray ECLDsp */
    StoreArray<ECLPureCsIInfo> m_eclPureCsIInfo; /**< storearray ECLPureCsIInfo - Special information for pure CsI simulation */

    double m_timeInverseSlope; /**< Time calibration inverse slope "a". */

    double m_pureCsIEnergyCalib = 0.00005; /**< conversion factor from ADC counts to GeV. */
    double m_pureCsITimeCalib = 10.; /**< conversion factor from eclPureCsIDigitizer to ns. */
    double m_pureCsITimeOffset = 0.31; /**< ad-hoc offset correction for pureCsI timing/ */

    void initializeCalibration(); /**< reads calibration constants, performs checks, put them into a vector */
    void callbackCalibration(DBObjPtr<ECLCrystalCalib>& cal, std::vector<float>& constants,
                             std::vector<float>& constantsUnc); /**< reads calibration constants */

    double getT99(const int cellid, const double energy, const bool fitfailed, const int bgcount) const; /**< t99%. */
    int determineBackgroundECL(); /**< count out of time digits to determine baclground levels */

    const double c_timeResolutionForFitFailed  = 1.0e9; /**< Time resolution for failed fits". */
    const double c_timeForFitFailed            = 0.0; /**< Time for failed fits". */

    // new time calibration from Kim and Chris
    std::string m_fileBackgroundName; /**< Background filename. */
    TFile* m_fileBackground{nullptr}; /**< Background file. */
    TH1F* m_th1fBackground{nullptr}; /**< Background histogram. */

    const double c_pol2Var1 = 1684.0; /**< 2-order fit for p1 Var1 + Var2*bg + Var3*bg^2. */
    const double c_pol2Var2 = 3080.0; /**< 2-order fit for p1. */
    const double c_pol2Var3 = -613.9; /**< 2-order fit for p1. */
    double m_pol2Max; /**< Maximum of p1 2-order fit to limit values */
    const int c_nominalBG = 183; /**< Number of out of time digits at BGx1.0. */
    double m_averageBG; /**< Average dose per crystal calculated from m_th1dBackground */
    const double c_minT99 = 3.5; /**< The minimum t99 */

    bool m_simulatePure = 0; /**< Flag to set pure CsI simulation option */


    std::unique_ptr< Belle2::ECL::ECLTimingUtilities > ECLTimeUtil =
      std::make_unique<Belle2::ECL::ECLTimingUtilities>(); /**< ECL timing tools */

    // For the energy dependence correction to the time
    // t-t0 = p1 + pow( (p3/(amplitude+p2)), p4 ) + p5*exp(-amplitude/p6)      ("Energy dependence equation")
    // Only change the time walk function paramters if they change away from the below dummy values
    double m_energyDependenceTimeOffsetFitParam_p1 = -999;     /**< p1 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p2 = -999;     /**< p2 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p3 = -999;     /**< p3 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p4 = -999;     /**< p4 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p5 = -999;     /**< p5 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p6 = -999;     /**< p6 in "energy dependence equation" */

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

    /** Name of the ECLDspPureCsI.*/
    virtual const char* eclDspArrayName() const override
    { return "ECLDspsPureCsI" ; }

    /** PureCsI Name of the EventLevelClusteringInfoPureCsI.*/
    virtual const char* eventLevelClusteringInfoName() const override
    { return "EventLevelClusteringInfoPureCsI" ; }

  };

} // end Belle2 namespace
