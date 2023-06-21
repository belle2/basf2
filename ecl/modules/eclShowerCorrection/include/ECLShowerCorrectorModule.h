/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dbobjects/ECLnOptimal.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>
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

    /** EventLevelClusteringInfo. */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo;

    /** location of cluster; cellID and position within the crystal*/
    ECL::ECLLeakagePosition* m_leakagePosition{nullptr};

    /** Leakage correction from database: leakage correction as a function of position */
    DBObjPtr<ECLLeakageCorrections> m_eclLeakageCorrections;
    TH2F thetaCorrection; /**< histogram of theta-dependent corrections */
    TH2F phiCorrection; /**< histogram of phi-dependent corrections */

    //..Parameters derived from payload (except for nThetaID & nLeakReg, which are fixed)
    const unsigned int nThetaID = 69; /**< 69 thetaIDs */
    static constexpr unsigned int nLeakReg = 3; /**< 0 = forward, 1 = barrel, 2 = backward */
    int nPositionBins = 0; /**< number of locations across crystal */
    int nXBins = 0; /**< number of thetaID x energy bins  */
    int nEnergies = 0; /**< number of energies for which there are leakage corrections */
    std::vector< std::vector<float> > leakLogE; /**< log(E) values for each region */

    /** nOptimal payload: bias from beam background, and correction from number of crystals */
    DBObjPtr<ECLnOptimal> m_eclNOptimal;
    TH2F m_bias; /**< 2D histogram of bias = sum of ECLCalDigit energy minus true (GeV) */
    TH2F m_logPeakEnergy; /**< log of peak energy (GeV) contained in nOptimal crystals */
    TH2F m_peakFracEnergy; /**< 2D histogram of peak fractional contained energy */

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
