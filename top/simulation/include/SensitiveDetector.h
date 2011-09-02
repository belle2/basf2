/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVEDETECTOR_H
#define TOPSENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <top/dataobjects/TOPSimHit.h>

namespace Belle2 {
  namespace top {
    //! The Class for TOP Sensitive Detector
    /*! In this class, every variable defined in TOPSimHit will be calculated,
      and stored in datastore.
    */
    class SensitiveDetector : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       * @param name Name of the sensitive detector. Do we still need that?
       */
      SensitiveDetector();

      /**
       * Process each step and calculate variables defined in PXDSimHit.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);


    private:


    }; // SensitiveDetector class
  } // end of namespace top
} // end of namespace Belle2

#endif /* TOPSENSITIVEDETECTOR_H */
