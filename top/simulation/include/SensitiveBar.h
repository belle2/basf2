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
      bool step(G4Step* aStep, G4TouchableHistory*);

      /**
       * Sets replica depth of module volume
       * @param depth replica depth
       */
      void setReplicaDepth(int depth) {m_replicaDepth = depth;}

    private:

      int m_replicaDepth = 2; /**< replica depth of module volume */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< geometry parameters */
      std::vector<int> m_trackIDs; /**< track ID's */

    };

  } // end of namespace top
} // end of namespace Belle2


