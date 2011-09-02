/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVEQUARTZ_H
#define TOPSENSITIVEQUARTZ_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  namespace top {
    //! This is optional (temporary) class that provides information on track parameters on aerogel plane, until tracking is not prepared.

    class SensitiveQuartz : public Simulation::SensitiveDetectorBase {

    public:


      /**
       * Constructor.
       * @param name Name of the sensitive detector. Do we still need that?
       */
      SensitiveQuartz();

      /**
       * Process each step and calculate variables defined in PXDSimHit.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);

    };

  } // end of namespace top
} // end of namespace Belle2

#endif /* TOPSENSITIVEQUARTZ_H */
