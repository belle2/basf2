/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSENSITIVEDETECTOR_H_
#define ECLSENSITIVEDETECTOR_H_

#include "G4VSensitiveDetector.hh"

namespace Belle2 {

  //! The Class for ECL Sensitive Detector
  class ECLSensitiveDetector: public G4VSensitiveDetector {

  public:

    //! Constructor
    ECLSensitiveDetector(G4String name);

    //! Destructor
    ~ECLSensitiveDetector();

    //! Register ECL hits collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in ECLHit (not yet prepared)
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

  protected:

  private:

  };

} // end of namespace Belle2

#endif /* ECLSENSITIVEDETECTOR_H_ */
