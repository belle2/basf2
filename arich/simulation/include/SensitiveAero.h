/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
