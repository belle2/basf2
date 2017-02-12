/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Oberhof                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef MCMATCHERECLCLUSTERSMODULE_H_
#define MCMATCHERECLCLUSTERSMODULE_H_

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {
  namespace ECL {

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
      StoreArray<ECLShower> m_eclShowers; /**< ECLShowers StoreArray*/
      StoreArray<ECLCluster> m_eclClusters; /**< ECLClusters StoreArray*/
      RelationArray m_eclHitToSimHitRelationArray; /**< ECLHits to ECLSimHits RelationArray*/
      RelationArray m_mcParticleToECLHitRelationArray; /**< MCParticles to ECLHits RelationArray*/
      RelationArray m_mcParticleToECLSimHitRelationArray; /**< MCParticles to ECLSimHits RelationArray*/
      RelationArray m_eclCalDigitToMCParticleRelationArray; /**< ECLCalDigits to MCParticles RelationArray*/
      RelationArray m_eclDigitToMCParticleRelationArray; /**< ECLDigits to MCParticles RelationArray*/
      RelationArray m_eclShowerToMCPart; /**< ECLShowera to MCParticles RelationArray*/
    };
  } //ECL
} // end of Belle2 namespace

#endif // MCMATCHERECLCLUSTERSMODULE_H_
