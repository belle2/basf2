/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDSENSITIVEDETECTOR_H_
#define PXDSENSITIVEDETECTOR_H_

#include <pxd/simpxd/PXDB4VHit.h>
#include "G4VSensitiveDetector.hh"

namespace Belle2 {

  //! The Class for PXD Sensitive Detector
  /*! In this class, every variable defined in PXDB4VHit will be calculated,
      and the PXDB4VHit will be added into a collection.
  */
  class PXDSensitiveDetector: public G4VSensitiveDetector {

  public:

    //! Constructor
    PXDSensitiveDetector(G4String name);

    //! Destructor
    ~PXDSensitiveDetector();

    //! Register PXDB4VHit collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in PXDB4VHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);


  protected:


  private:

    PXDB4VHitsCollection* m_hitCollection; /*!< The collection of hits in this sensitive detector. */
    G4int m_hitColID;

    PXDB4VHit* m_currentHit;

  };

} // end of namespace Belle2

#endif /* PXDSENSITIVEDETECTOR_H_ */
