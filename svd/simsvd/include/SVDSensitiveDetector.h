/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSENSITIVEDETECTOR_H_
#define SVDSENSITIVEDETECTOR_H_

#include <svd/hitsvd/SVDSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  /**
   * The SVD Sensitive Detector class.
   *
   * In this class, every variable defined in SVDSimHit will be
   * calculated, and the SVDSimHit will be added into  collection.
   * This is a simplistic implementation usable only for the very
   * thin SVD detectors - a single GEANT4 step is assumed for each
   * pass through the active detector.
   */

  class SVDSensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    /** Constructor. */
    SVDSensitiveDetector(G4String name);

    //! Destructor
    ~SVDSensitiveDetector();

    //! Process each step and calculate variables defined in SVDB4VHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);


  protected:


  private:

  };

} // end of namespace Belle2

#endif /* SVDSENSITIVEDETECTOR_H_ */
