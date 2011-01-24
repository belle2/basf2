/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDSENSITIVEDETECTOR_H_
#define PXDSENSITIVEDETECTOR_H_

#include <pxd/hitpxd/PXDSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  /**
   * The PXD Sensitive Detector class.
   *
   * In this class, every variable defined in PXDSimHit will be
   * calculated, and the PXDSimHit will be added to the DataStore.
   * This is a simplistic implementation usable only for the very
   * thin PXD detectors - a single GEANT4 step is assumed for each
   * pass through the active detector.
   */

  class PXDSensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    /** Constructor. */
    PXDSensitiveDetector(G4String name);

    //! Destructor
    ~PXDSensitiveDetector();

    //! Process each step and calculate variables defined in PXDB4VHit
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);


  protected:


  private:

    int m_hitNumber; /**< The current number of created hits. Used to fill the DataStore PXD array.*/

  };

} // end of namespace Belle2

#endif /* PXDSENSITIVEDETECTOR_H_ */
