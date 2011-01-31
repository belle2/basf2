/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVEDETECTOR_H_
#define TOPSENSITIVEDETECTOR_H_

#include "simulation/kernel/SensitiveDetectorBase.h"

namespace Belle2 {

  //! The Class for TOP Sensitive Detector
  /*! In this class, every variable defined in TOPSimHit will be calculated,
      and stored in datastore.
  */
  class TOPSensitiveDetector : public Simulation::SensitiveDetectorBase {
  public:

    //! Constructor
    TOPSensitiveDetector(G4String name);

    //! Destructor
    ~TOPSensitiveDetector();

    //! Initialize sensitive detector
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in PXDB4VHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

  private:


  };
} // end of namespace Belle2

#endif /* TOPSENSITIVEDETECTOR_H_ */
