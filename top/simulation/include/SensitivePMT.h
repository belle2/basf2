/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <top/geometry/TOPGeometryPar.h>

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
      bool step(G4Step* aStep, G4TouchableHistory*);

    private:

      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< geometry parameters */

    };
  } // end of namespace TOP
} // end of namespace Belle2

