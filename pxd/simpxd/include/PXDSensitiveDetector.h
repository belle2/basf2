/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDSENSITIVEDETECTOR_H_
#define PXDSENSITIVEDETECTOR_H_

#include <pxd/dataobjects/PXDSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>
#include <TG4RootDetectorConstruction.h>

class G4Step;
class G4TouchableHistory;
class G4HCofThisEvent;

namespace Belle2 {

  /**
   * The PXD Sensitive Detector class.
   *
   * This class stores Geant4 steps (pieces of tracks) in the PXD acitve sensors
   * to PXDSimHits. The steps are not aggregated and are intended to be directly
   * digitized. The PXDSimHits are saved in a DataStore collection together with their
   * relations to MCParticles (actually, the relations are indexed by trackIDs rather than
   * MCParticles, the relations to MCParticles are later restored by the framework.
   * <p>
   * The threshold parameter defines minimum energy that a Geant4 step has to deposit to be saved in
   * the DataStore. It is a dimensionless number defining the fraction of a MIP deposition per unit
   * path length.
   * <p>
   * Detection of photons. The original version only stores hits from charged tracks, meaning
   * that only pair production is taken into account for photons. In this version, also
   * photoeffect and Compton scattering are taken into account.
   * <p>
   * Based on the implementation of the Mokka VXD sensitive detector class.
   * @author Z. Drasal, Charles University Prague (based on TRKSD00 sens. detector)
   * @author P. Kvasnicka, Charles University Prague (basf2 implementation)
   */

  class PXDSensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    /**
    * Constructor.
    *
    * @param name Name of the sensitive detector. Do we still need that?
    */
    PXDSensitiveDetector(G4String name);

    /** Destructor. */
    ~PXDSensitiveDetector();

    /** Initialize sensitive detector. */
    void Initialize(G4HCofThisEvent* HCTE);

    /**
    * Process each step and calculate variables defined in PXDSimHit.
    * @param step Current Geant4 step in the sensitive medium.
    * @result true if a hit was stored, o.w. false.
    */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*);

  private:

    G4double m_thresholdPXD;    /**< Threshold for deposited energy, a fraction of MIP depositon per unit length.
                   * If a hit deposits less, it is not stored.
                   */
    TG4RootDetectorConstruction* m_detMap; /**< Detector construction to provide
                   * link between G4 and TGeo volumes. */

  }; // PXDSensitiveDetector class

} // end of namespace Belle2

#endif /* PXDSENSITIVEDETECTOR_H_ */
