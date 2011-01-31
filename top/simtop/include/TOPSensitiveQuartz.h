/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVEQUARTZ_H_
#define TOPSENSITIVEQUARTZ_H_

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  //! This is optional (temporary) class that provides information on track parameters on quartz plane, until tracking is not prepared.

  class TOPSensitiveQuartz: public Simulation::SensitiveDetectorBase {

  public:

    //! Constructor
    TOPSensitiveQuartz(G4String name);

    //! Destructor
    ~TOPSensitiveQuartz();

    //! Register QuartzHit collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in QUARTZHitTOP
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

  };

} // end of namespace Belle2

#endif /* TOPSENSITIVEQUARTZ_H_ */
