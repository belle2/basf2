/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHSENSITIVEAERO_H
#define ARICHSENSITIVEAERO_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  namespace arich {
    //! This is optional (temporary) class that provides information on track parameters on aerogel plane, until tracking is not prepared.

    class SensitiveAero : public Simulation::SensitiveDetectorBase {

    public:


      /**
       * Constructor.
       * @param name Name of the sensitive detector. Do we still need that?
       */
      SensitiveAero();

      /**
       * Process each step and calculate variables defined in PXDSimHit.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*) override;

    };

  } // end of namespace arich
} // end of namespace Belle2

#endif /* ARICHSENSITIVEAERO_H */
