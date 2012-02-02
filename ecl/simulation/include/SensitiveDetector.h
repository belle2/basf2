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

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  //! The Class for ECL Sensitive Detector
  class SensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    //! Constructor
//    SensitiveDetector(G4String name);
    SensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);
    //! Destructor
    ~SensitiveDetector();

    //! Register ECL hits collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent* HCTE);

    //! Process each step and calculate variables defined in ECLHit (not yet prepared)
    bool step(G4Step* aStep, G4TouchableHistory* history);
//    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent* eventHC);


    //! Save ECLSimHit into datastore
    int saveSimHit(
      const G4int cellId,
      const G4int thetaId,
      const G4int phiId,
      const G4int trackID,
      const G4int pid,
      const G4double tof,
      const G4double edep,
      const G4double stepLength,
      const G4ThreeVector& mom,
      const G4ThreeVector& posW,
      const G4ThreeVector& posIn,
      const G4ThreeVector& posOut
    );

    //! Save EBECLSimHit into datastore
    int saveEBSimHit(
      const G4int cellId,
      const G4int thetaId,
      const G4int phiId,
      const G4int trackID,
      const G4int pid,
      const G4double tof,
      const G4double edep,
      const G4double stepLength,
      const G4ThreeVector& mom,
      const G4ThreeVector& posW,
      const G4ThreeVector& posIn,
      const G4ThreeVector& posOut
    );


    //! Get cell, theta, phi Id from PhysicalVolume
    int Mapping(const G4String VolumeName);
    int m_phiID; /**< The current phi ID in an event. Used to fill the DataStore ECL array.*/
    int m_thetaID; /**< The current theta ID in an event. Used to fill the DataStore ECL array.*/
    int m_cellID; /**< The current cellID in an event. Used to fill the DataStore ECL array.*/
  protected:

  private:

    G4double m_thresholdEnergyDeposit;/**Energy Deposit  threshold  */
    G4double m_thresholdKineticEnergy;/**Kinetic Energy  threshold  */
    int m_hitNumber; /**< The current number of created hits in an event. Used to fill the DataStore ECL array.*/
    int m_EBhitNumber; /**< The current number of created hits in an event. Used to fill the DataStore ECL EB array.*/
    int oldtrack;
    int oldcellId;

  };

} // end of namespace Belle2

#endif /* ECLSENSITIVEDETECTOR_H_ */
