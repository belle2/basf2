/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroshi Nakano, Andreas Moll                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef IRSENSITIVEDETECTOR_H
#define IRSENSITIVEDETECTOR_H

#include <ir/dataobjects/IRSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>
#include <TG4RootDetectorConstruction.h>

class G4Step;
class G4TouchableHistory;
class G4HCofThisEvent;

namespace Belle2 {

  /**
   * The IR Sensitive Detector class.
   * This class stores Geant4 steps (pieces of tracks) in the IR active sensors.
   */

  class IRSensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    /**
    * Constructor.
    *
    * @param name Name of the sensitive detector.
    */
    IRSensitiveDetector(G4String name);

    /** Destructor. */
    ~IRSensitiveDetector();

    /** Initialize sensitive detector. */
    void Initialize(G4HCofThisEvent* HCTE);

    /**
    * Process each step.
    * @param step Current Geant4 step in the sensitive medium.
    * @result true if a hit was stored, o.w. false.
    */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*);

  private:

  }; // IRSensitiveDetector class

} // end of namespace Belle2

#endif /* IRSENSITIVEDETECTOR_H */
