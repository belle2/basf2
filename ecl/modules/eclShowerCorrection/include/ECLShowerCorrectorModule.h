/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower leakage              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Alon Hershenhorn (hersehn@physics.ubc.ca)                *
 *               Suman Koirala (suman@ntu.edu.tw)                         *
 *               Christopher Hearty (hearty@physics.ubc.ca                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <TH2F.h>

namespace Belle2 {
  class ECLLeakageCorrections;
  class ECLShower;

  namespace ECL {
    class ECLLeakagePosition;
  }

  /** Class to perform the shower correction */
  class ECLShowerCorrectorModule : public Module {

  public:
    /** Constructor. */
    ECLShowerCorrectorModule();

    /** Destructor. */
    ~ECLShowerCorrectorModule();

    /** Initialize. */
    virtual void initialize() override;

    /** Begin run. */
    virtual void beginRun() override;

    /** Event. */
    virtual void event() override;

    /** End run. */
    virtual void endRun() override;

    /** Terminate. */
    virtual void terminate() override;

  private:

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** location of cluster; cellID and position within the crystal*/
    ECL::ECLLeakagePosition* leakagePosition{nullptr};

    /** Leakage correction from database */
    DBObjPtr<ECLLeakageCorrections> m_eclLeakageCorrections;
    TH2F thetaCorrection; /**< histogram of theta-dependent corrections */
    TH2F phiCorrection; /**< histogram of phi-dependent corrections */
    TH2F crysCorrection; /**< histogram of nCrys-dependent corrections */

    //..Parameters derived from payload (except for nThetaID & nLeakReg, which are fixed)
    const int nThetaID = 69; /**< 69 thetaIDs */
    const int nLeakReg = 3; /**< 0 = forward, 1 = barrel, 2 = backward */
    int nPositionBins = 0; /**< number of locations across crystal */
    int nXBins = 0; /**< number of thetaID x energy bins  */
    int nCrysMax = 0; /**< max number of crystals used to calculate energy */
    int nEnergies = 0; /**< number of energies for which there are leakage corrections */
    std::vector< std::vector<float> > leakLogE; /**< log(E) values for each region */

  public:
    /** We need names for the data objects to differentiate between PureCsI and default*/

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

  };

  /** The very same module but for PureCsI */
  class ECLShowerCorrectorPureCsIModule : public ECLShowerCorrectorModule {
  public:

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

  };

} // end of Belle2 namespace
