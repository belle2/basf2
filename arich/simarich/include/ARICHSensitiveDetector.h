/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHSENSITIVEDETECTOR_H_
#define ARICHSENSITIVEDETECTOR_H_

#include "simulation/kernel/SensitiveDetectorBase.h"

namespace Belle2 {

  //! The Class for ARICH Sensitive Detector
  /*! In this class, every variable defined in ARICHSimHit will be calculated,
      and stored in datastore.
  */
  class ARICHSensitiveDetector : public Simulation::SensitiveDetectorBase {
  public:

    //! Constructor
    ARICHSensitiveDetector(G4String name);

    //! Destructor
    ~ARICHSensitiveDetector();

    //! Initialize sensitive detector
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in PXDB4VHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

  private:


  };
} // end of namespace Belle2

#endif /* ARICHSENSITIVEDETECTOR_H_ */
