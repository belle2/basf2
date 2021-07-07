/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  namespace ir {
    /**
     * The IR Sensitive Detector class.
     *
     * This class stores Geant4 steps (pieces of tracks) in the IR acitve sensors
     * to IRSimHits. The steps are not aggregated and are intended to be directly
     * digitized. The IRSimHits are saved in a DataStore collection together with their
     * relations to MCParticles (actually, the relations are indexed by trackIDs rather than
     * MCParticles, the relations to MCParticles are later restored by the framework.
     *
     * The threshold parameter defines minimum energy that a Geant4 step has to deposit to be saved in
     * the DataStore. It is a dimensionless number defining the fraction of a MIP deposition per unit
     * path length.
     *
     * Detection of photons. The original version only stores hits from charged tracks, meaning
     * that only pair production is taken into account for photons. In this version, also
     * photoeffect and Compton scattering are taken into account.
     *
     * Based on the implementation of the Mokka VXD sensitive detector class.
     * @author Z. Drasal, Charles University Prague (based on TRKSD00 sens. detector)
     * @author P. Kvasnicka, Charles University Prague (basf2 implementation)
     */

    class SensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       * @param sensorInfo SensorInfo instance of the Sensor
       */
      SensitiveDetector();

      /**
       * Process each step and calculate variables defined in IRSimHit.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*) override;

    private:

    }; // SensitiveDetector class
  } //end of namespace IR
} // end of namespace Belle2
