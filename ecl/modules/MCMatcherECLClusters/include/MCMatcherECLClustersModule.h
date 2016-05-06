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
#include <ecl/dataobjects/ECLHitAssignment.h>
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
      StoreArray<MCParticle> m_mcParticles;
      StoreArray<ECLHit> m_eclHits;
      StoreArray<ECLCalDigit> m_eclCalDigits;
      StoreArray<ECLDigit> m_eclDigits;
      StoreArray<ECLSimHit> m_eclSimHits;
      StoreArray<ECLShower> m_eclShowers;
      StoreArray<ECLHitAssignment> m_eclHitAssignments;
      StoreArray<ECLCluster> m_eclClusters;
      RelationArray m_eclHitToSimHitRelationArray;
      RelationArray m_mcParticleToECLHitRelationArray;
      RelationArray m_mcParticleToECLSimHitRelationArray;
      RelationArray m_eclCalDigitToMCParticleRelationArray;
      RelationArray m_eclDigitToMCParticleRelationArray;
      RelationArray m_eclShowerToMCPart;
    };
  } //ECL
} // end of Belle2 namespace

#endif // MCMATCHERECLCLUSTERSMODULE_H_
