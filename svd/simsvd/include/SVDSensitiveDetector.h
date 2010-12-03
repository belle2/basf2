/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSENSITIVEDETECTOR_H_
#define SVDSENSITIVEDETECTOR_H_

#include <svd/simsvd/SVDB4VHit.h>
#include "G4VSensitiveDetector.hh"

namespace Belle2 {

  /**
   * The SVD Sensitive Detector class.
   *
   * In this class, every variable defined in SVDB4VHit will be
   * calculated, and the SVDB4VHit will be added into  collection.
   * This is a simplistic implementation usable only for the very
   * thin SVD detectors - a single GEANT4 step is assumed for each
   * pass through the active detector.
   */

  class SVDSensitiveDetector: public G4VSensitiveDetector {

  public:

    /** Constructor. */
    SVDSensitiveDetector(G4String name);

    //! Destructor
    ~SVDSensitiveDetector();

    //! Register SVDB4VHit collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in SVDB4VHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);


  protected:


  private:

    SVDB4VHitsCollection* m_hitCollection; /*!< The collection of hits in this sensitive detector. */
    G4int m_hitColID;

    SVDB4VHit* m_currentHit;

  };

} // end of namespace Belle2

#endif /* SVDSENSITIVEDETECTOR_H_ */
