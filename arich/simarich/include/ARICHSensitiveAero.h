/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHSENSITIVEAERO_H
#define ARICHSENSITIVEAERO_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  //! This is optional (temporary) class that provides information on track parameters on aerogel plane, until tracking is not prepared.

  class ARICHSensitiveAero: public Simulation::SensitiveDetectorBase {

  public:

    //! Constructor
    ARICHSensitiveAero(G4String name);

    //! Destructor
    ~ARICHSensitiveAero();

    //! Register AeroHit collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in ARICHAeroHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

  };

} // end of namespace Belle2

#endif /* ARICHSENSITIVEAERO_H */
