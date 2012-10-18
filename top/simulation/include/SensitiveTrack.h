/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVETRACK_H
#define TOPSENSITIVETRACK_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <top/geometry/TOPGeometryPar.h>


namespace Belle2 {
  namespace TOP {

    //! Optional (temporary) class providing the information on MCParticle at TOP bars

    class SensitiveTrack : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       */
      SensitiveTrack();

      /**
       * Process each step and fill TOPBarHits
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);

    protected:

      TOPGeometryPar* m_topgp; /**< geometry parameters from xml */

    };

  } // end of namespace top
} // end of namespace Belle2

#endif

