/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <top/geometry/TOPGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>


namespace Belle2 {
  namespace TOP {

    /**
     * Class providing information on MCParticles hitting the bars.
     * Applies also quantum efficiency to reduce the number of propagating photons.
     */
    class SensitiveBar : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       */
      SensitiveBar();

      /**
       * Process each step and fill TOPBarHits
       * @param aStep Current Geant4 step in the sensitive medium.
       * @return true when particle that is not an optical photon enters the bar
       */
      bool step(G4Step* aStep, G4TouchableHistory*) override;

      /**
       * Sets replica depth of module volume
       * @param depth replica depth
       */
      void setReplicaDepth(int depth) {m_replicaDepth = depth;}

    private:

      int m_replicaDepth = 2; /**< replica depth of module volume */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< geometry parameters */
      std::vector<int> m_trackIDs; /**< track ID's */

      StoreArray<MCParticle> m_mcParticles; /**< collection of MC particles */
      StoreArray<TOPBarHit>  m_barHits; /**< collection of entrance-to-bar hits */
      RelationArray m_relParticleHit {m_mcParticles, m_barHits}; /**< relations */

    };

  } // end of namespace top
} // end of namespace Belle2


