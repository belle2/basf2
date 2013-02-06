/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSENSITIVEDETECTOR_H_
#define ECLSENSITIVEDETECTOR_H_

#include <ecl/geometry/ECLGeometryPar.h>
#include <simulation/kernel/SensitiveDetectorBase.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "TVector3.h"

using namespace std;

namespace Belle2 {
  namespace ECL {
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
        const G4int trackID,
        const G4int pid,
        const G4double tof,
        const G4double edep,
        G4ThreeVector mom,
        TVector3 WightedPos
      );



      //! Get cell, theta, phi Id from PhysicalVolume
      int Mapping(const G4String VolumeName);
    protected:

    private:




      /** members of  SensitiveDetector  */
      G4double m_thresholdEnergyDeposit;/** Energy Deposit  threshold  */
      G4double m_thresholdKineticEnergy;/** Kinetic Energy  threshold  */
      int m_simhitNumber;                  /** The current number of created hits in an event. Used to fill the DataStore ECLSimHit. */
      int m_hitNum;                  /** The current number of created hits in an event. Used to fill the DataStore ECLHit.*/
      int m_EvnetNumber;                /**  The current number of created hits in an event. Used to fill the DataStore ECL EB array. */
      int m_oldEvnetNumber;                /**  The current number of created hits in an event. Used to fill the DataStore  */
      int m_trackID;                    /** track id */
      G4ThreeVector m_startPos;         /**  Position of prestep */
      G4ThreeVector m_endPos;     /**  Position of poststep*/
      TVector3 m_WightedPos;      /**  Wighted step Position*/
      G4ThreeVector m_momentum;   /**  momentum of track */
      double m_startTime;         /** global time */
      double m_endTime;         /** global time */
      double m_WightedTime;         /** global time */
      double m_startEnergy;       /** particle energy at the entrance in volume */
      double m_energyDeposit;     /** energy deposited in volume */
      double m_trackLength;       /** length of the track in the volume */

      int ECLHitIndex[8736][80]; /** Hit index of StoreArray */
      int iECLCell;              /** Hit Energy of StoreArray */
      int TimeIndex;             /** Hit Time of StoreArray */
      TVector3 PosCell;          /** center of crystal position */
      TVector3 VecCell;          /** vector of crystal axis */
      double local_pos;          /** position alongthe vector of crystal axis   */
      double T_ave;              /** flight time to diode sensor  */
      int firstcall;             /** flag of first call   */
      int m_phiID;               /** The current phi ID in an event. Used to fill the DataStore ECL array  */
      int m_thetaID;             /** The current theta ID in an event. Used to fill the DataStore ECL array  */
      int m_cellID;              /** The current cellID in an event. Used to fill the DataStore ECL array  */
      typedef std::map< int, int>  PrimaryTrackMap; /** define a map type for Primary Track  */
      PrimaryTrackMap eclPrimaryMap; /** the map to store Primary Track  */

    };
  } // end of namespace ecl
} // end of namespace Belle2

#endif /* ECLSENSITIVEDETECTOR_H_ */
