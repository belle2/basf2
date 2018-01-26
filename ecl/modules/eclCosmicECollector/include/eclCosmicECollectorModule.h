/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty hearty@physics.ubc.ca                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCOSMICECOLLECTORMODULE_H
#define ECLCOSMICECOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLDigit.h>

namespace Belle2 {

  /** class eclCosmicECollectorModule. Select real or MC cosmic ray events for ecl
   * single crystal energy calibration and accumulate normalized energy distributions
   * for each crystal */
  class eclCosmicECollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor. */
    eclCosmicECollectorModule();

    /** prepare. Define histograms to be filled and stored; ecl geometry calculations */
    void prepare();

    /** collect. Select events and fill histograms */
    void collect();

  private:

    /** Parameters to control the job */
    double m_minCrysE; /**< Minimum energy for a crystal to be considered hit (0.01 GeV) */
    bool m_mockupL1; /**< Calculate energy per trigger cell in lieu of trigger simulation (false) */
    double m_trigThreshold; /**< Minimum energy in trigger cell, if required (0.1 GeV) */

    int iEvent = 0; /**< event counter */

    StoreArray<ECLDigit> eclDigitArray; /**< Required input array of eclDigits */


    /** Sets of three crystals that define a useful cosmic */
    std::vector<short int> CenterCrys; /**< crystal ID whose signal will be recorded */
    std::vector<short int> NeighbourA; /**< if this crystal is > threshold */
    std::vector<short int> NeighbourB; /**< and this crystal is > threshold */
    std::vector<int> FirstSet; /**< First set of 3 crystals for each crystalID */

    /** Energy related */
    std::vector<float> EperCrys; /**< ECL digit energy as a function of crystalID */
    std::vector<bool> HitCrys; /**< true if energy>m_minCrysE */
    std::vector<float> EnergyPerTC; /**< Energy (GeV) per trigger cell */

    /** Trigger */
    std::vector<short int> TCperCrys; /**< trigger cell minus 1 for each crystalID */

    /** Expected energies from database; neighbours in the same ThetaID ring */
    DBObjPtr<ECLCrystalCalib> m_ECLExpCosmicESame; /**< database object */
    std::vector<float> ExpCosmicESame; /**< vector obtained from DB object */

    /** Expected energies from database; neighbours in different ThetaID rings */
    DBObjPtr<ECLCrystalCalib> m_ECLExpCosmicEDifferent; /**< database object */
    std::vector<float> ExpCosmicEDifferent; /**< vector obtained from DB object */

    /** Electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsCalib; /**< database object */
    std::vector<float> ElectronicsCalib; /**< vector obtained from DB object */

    /** Existing single crystal calibration from DB; will be updated by CAF */
    DBObjPtr<ECLCrystalCalib> m_CosmicECalib; /**< database object */
    std::vector<float> CosmicECalib; /**< vector obtained from DB object */

    //** Time offset from electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsTime; /**< database object */
    std::vector<float> ElectronicsTime; /**< vector obtained from DB object */

    //** Time offset from cosmic calibration from database */
    DBObjPtr<ECLCrystalCalib> m_TimeOffset; /**< database object */
    std::vector<float> TimeOffset; /**< vector obtained from DB object */
  };
}

#endif /* ECLCOSMICECOLLECTORMODULE_H */
