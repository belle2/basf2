/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Oberhof                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

namespace Belle2 {
  class MCParticle;
  class ECLHit;
  class ECLCalDigit;
  class ECLDigit;
  class ECLSimHit;
  class ECLCluster;
  class ECLShower;

  /** Class to represent the hit of one cell */
  class MCMatcherECLClustersModule : public Module {

  public:
    /** Constructor.*/
    MCMatcherECLClustersModule();

    /** Destructor.*/
    virtual ~MCMatcherECLClustersModule();

    /** Initialize variables, print info, and start CPU clock. */
    virtual void initialize();

    /** Nothing so far.*/
    virtual void beginRun();

    /** Actual digitization of all hits in the ECL.
     *
     */
    virtual void event();

    /** Nothing so far. */
    virtual void endRun();

    /** Stopping of CPU clock.*/
    virtual void terminate();

  private:

    StoreArray<MCParticle> m_mcParticles; /**< MCParticles StoreArray*/
    StoreArray<ECLHit> m_eclHits; /**< ECLHits StoreArray*/
    StoreArray<ECLCalDigit> m_eclCalDigits; /**< ECLCalDigits StoreArray*/
    StoreArray<ECLDigit> m_eclDigits; /**< ECLDigits StoreArray*/
    StoreArray<ECLSimHit> m_eclSimHits; /**< ECLSimHits StoreArray*/
    StoreArray<ECLCluster> m_eclClusters; /**< ECLClusters StoreArray*/
    StoreArray<ECLShower> m_eclShowers; /**< ECLShowers StoreArray*/

    /** Default name ECLDigits */
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    /** Default name ECLCalDigits */
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Default name ECLClusters */
    virtual const char* eclClusterArrayName() const
    { return "ECLClusters" ; }

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    RelationArray m_mcParticleToECLHitRelationArray; /**< MCParticles to ECLHits RelationArray*/

    RelationArray m_mcParticleToECLSimHitRelationArray; /**< MCParticles to ECLSimHits RelationArray*/

  };

  /** Same module but for pure CsI */
  class MCMatcherECLClustersPureCsIModule : public MCMatcherECLClustersModule {

  public:

    /** PureCsI name ECLDigitsPureCsI */
    virtual const char* eclDigitArrayName() const override
    { return "ECLDigitsPureCsI" ; }

    /** PureCsI name ECLCalDigitsPureCsI */
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** PureCsI name ECLClustersPureCsI */
    virtual const char* eclClusterArrayName() const override
    { return "ECLClustersPureCsI" ; }

  };

} // end of Belle2 namespace
