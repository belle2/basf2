/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower (mainly longitudinal *
 * leakage): corr = (Reconstructed / Truth).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Alon Hershenhorn (hersehn@physics.ubc.ca)                *
 *               Suman Koirala (suman@ntu.edu.tw)                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// ECL
#include <ecl/dataobjects/ECLShower.h>

#include <ecl/dataobjects/ECLEventInformation.h>
#include <ecl/dbobjects/ECLShowerCorrectorLeakageCorrection.h>
#include <ecl/dbobjects/ECLShowerEnergyCorrectionTemporary.h>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// OTHER
#include <string>
#include <vector>
#include <TGraph2D.h>


namespace Belle2 {

  /** Class to perform the shower correction */
  class ECLShowerCorrectorModule : public Module {

  public:
    /** Constructor. */
    ECLShowerCorrectorModule();

    /** Destructor. */
    ~ECLShowerCorrectorModule();

    /** Initialize. */
    virtual void initialize();

    /** Begin run. */
    virtual void beginRun();

    /** Event. */
    virtual void event();

    /** End run. */
    virtual void endRun();

    /** Terminate. */
    virtual void terminate();

    /** Prepare correction */
    void prepareLeakageCorrections();

    /** Get correction for BGx0*/
    double getLeakageCorrection(const double theta, const double phi, const double energy, const double background) const;

    /** Get correction for BGx1 (temporary)*/
    double getLeakageCorrectionTemporary(const double theta, const double energy, const double background);

  private:
    DBObjPtr<ECLShowerCorrectorLeakageCorrection> m_leakageCorrectionPtr_bgx0;  /**< Leakage corrections from DB for BGx0 */
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_phase2bgx1;  /**< Leakage corrections from DB for Phase2 BG15x1.0*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_phase3bgx1;  /**< Leakage corrections from DB for Phase3 BG15x1.0 */

    const double m_fullBkgdCount = 183.0; /**< Nominal Background at BGx1.0 (MC12) */

    // For temporary BGx1
    TGraph2D m_leakage_bgx1[2]; // 0: phase2, 1: phase3
    std::vector<double> m_leakage_bgx1_limits[2];

    // For BGx0 (complicated theta and phi dependency)
    // Vectors with one entry each:
    int m_numOfBfBins; /**< number of background fraction bins; currently only two */
    int m_numOfEnergyBins; /**< number of energy bins */
    int m_numOfPhiBins; /**< number of phi bins */
    int m_numOfReg1ThetaBins; /**< number of region 1 theta bins */
    int m_numOfReg2ThetaBins; /**< number of region 2 theta bins */
    int m_numOfReg3ThetaBins; /**< number of region 3 theta bins */
    int m_phiPeriodicity; /**< repeating pattern in phi direction, for barrel it is 72 */
    float m_lReg1Theta; /**< lower boundary of the region 1 theta */
    float m_hReg1Theta; /**< upper boundary of the region 1 theta */
    float m_lReg2Theta; /**< lower boundary of the region 2 theta */
    float m_hReg2Theta; /**< upper boundary of the region 2 theta */
    float m_lReg3Theta; /**< lower boundary of the region 3 theta */
    float m_hReg3Theta; /**< upper boundary of the region 3 theta */

    // Vector with (right now) 15 entries
    std::vector<float> m_avgRecEn; /**< averages of the energy bins */

    // Vectors with all corrections
    std::vector<int> m_bgFractionBinNum; /**< BG fraction bin */
    std::vector<int> m_regNum; /**< region bin */
    std::vector<int> m_phiBinNum; /**< phi bin */
    std::vector<int> m_thetaBinNum; /**< theta bin*/
    std::vector<int> m_energyBinNum; /**< energu bin */
    std::vector<float> m_correctionFactor; /**< correction value*/

    std::vector < std::vector < std::vector < std::vector < float > > > > m_reg1CorrFactorArrays; /**< region 1 corrections */
    std::vector < std::vector < std::vector < std::vector < float > > > > m_reg2CorrFactorArrays; /**< region 2 corrections */
    std::vector < std::vector < std::vector < std::vector < float > > > > m_reg3CorrFactorArrays; /**< region 3 corrections */

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** Store object pointer: ECLEventInformation. */
    StoreObjPtr<ECLEventInformation> m_eclEventInformation;

  public:
    /** We need names for the data objects to differentiate between PureCsI and default*/

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Name to be used for default option: ECLEventInformation.*/
    virtual const char* eclEventInformationName() const
    { return "ECLEventInformation" ; }

  };

  /** The very same module but for PureCsI */
  class ECLShowerCorrectorPureCsIModule : public ECLShowerCorrectorModule {
  public:

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** Name to be used for PureCsI option: ECLEventInformationPureCsI.*/
    virtual const char* eclEventInformationName() const override
    { return "ECLEventInformationPureCsI" ; }

  };

} // end of Belle2 namespace
