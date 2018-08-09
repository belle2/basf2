/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef HE3TUBE_SENSITIVEDETECTOR_H
#define HE3TUBE_SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the HE3TUBE detector */
  namespace he3tube {

    /** Sensitive Detector implementation of the HE3TUBE detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector();

      //! Save saveG4TrackInfo into datastore
      int saveG4TrackInfo(
        int trackID,
        int PDG,
        float Mass,
        float Energy,
        float vtx[3],
        float mom[3]
      );

    protected:
      /** Step processing method
       * @param step the G4Step with the current step information
       * @return true if a Hit has been created, false if the hit was ignored
       */
      bool step(G4Step* step, G4TouchableHistory*);
      int m_trackID;                    /** track id */
    };

  } //he3tube namespace
} //Belle2 namespace

#endif
