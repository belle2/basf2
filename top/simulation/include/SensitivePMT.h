/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <top/geometry/TOPGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPSimPhoton.h>


namespace Belle2 {
  namespace TOP {
    /**
     * Class providing SimHits.
     */
    class SensitivePMT : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       */
      SensitivePMT();

      /**
       * Process each step, fill TOPSimHits and TOPSimPhotons
       * @param aStep Current Geant4 step in the sensitive medium.
       * @return true if optical photon detected.
       */
      bool step(G4Step* aStep, G4TouchableHistory*) override;

      /**
       * Sets replica depth of PMT volume
       * @param depth replica depth
       */
      void setPMTReplicaDepth(int depth) {m_pmtReplicaDepth = depth;}

      /**
       * Sets replica depth of module volume
       * @param depth replica depth
       */
      void setModuleReplicaDepth(int depth) {m_moduleReplicaDepth = depth;}

    private:

      int m_pmtReplicaDepth = 2;    /**< replica depth of PMT volume */
      int m_moduleReplicaDepth = 5; /**< replica depth of module volume */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< geometry parameters */

      StoreArray<MCParticle> m_mcParticles; /**< collection of MC particles */
      StoreArray<TOPSimHit> m_simHits; /**< collection of simulated hits */
      StoreArray<TOPSimPhoton> m_simPhotons; /**< collection of simulated photons */
      RelationArray m_relParticleHit {m_mcParticles, m_simHits}; /**< relations */

    };
  } // end of namespace TOP
} // end of namespace Belle2

