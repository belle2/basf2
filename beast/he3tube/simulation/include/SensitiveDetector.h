/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      bool step(G4Step* step, G4TouchableHistory*) override;
      /** track id */
      int m_trackID;
    };

  } //he3tube namespace
} //Belle2 namespace

#endif
