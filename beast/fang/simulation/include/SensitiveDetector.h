/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef FANG_SENSITIVEDETECTOR_H
#define FANG_SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <G4EmSaturation.hh>
namespace Belle2 {
  /** Namespace to encapsulate code needed for the FANG detector */
  namespace fang {

    /** Sensitive Detector implementation of the FANG detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector();
    protected:
      /** Step processing method
       * @param step the G4Step with the current step information
       * @return true if a Hit has been created, false if the hit was ignored
       */
      bool step(G4Step* step, G4TouchableHistory*);
    private:
      /** needed to call Birk's law*/
      G4EmSaturation* saturationEngine;
    };

  } //fang namespace
} //Belle2 namespace

#endif
