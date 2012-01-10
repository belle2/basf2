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
  namespace TOP {
    //! The Class for TOP Sensitive PMT
    /*! In this class, variables will be calculated that will be later stored in TOPSimHit
    */
    class SensitivePMT : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       */
      SensitivePMT();

      /**
       * Process each step and calculate variables defined in TOPSimHit.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @return true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);


    private:


    }; // SensitivePMT class
  } // end of namespace top
} // end of namespace Belle2

#endif /* TOPSENSITIVEDETECTOR_H */
