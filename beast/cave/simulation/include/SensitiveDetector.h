/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CAVE_SENSITIVEDETECTOR_H
#define CAVE_SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CAVE detector */
  namespace cave {

    /** Sensitive Detector implementation of the CAVE detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector();
    protected:
      /** Step processing method
       * @param step the G4Step with the current step information
       * @return true if a Hit has been created, false if the hit was ignored
       */
      bool step(G4Step* step, G4TouchableHistory*) override;
    };

  } //cave namespace
} //Belle2 namespace

#endif
