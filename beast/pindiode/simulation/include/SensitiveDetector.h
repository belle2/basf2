/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PINDIODE_SENSITIVEDETECTOR_H
#define PINDIODE_SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PINDIODE detector */
  namespace pindiode {

    /** Sensitive Detector implementation of the PINDIODE detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector();
      /** Destructor*/
      ~SensitiveDetector();

      //! Save PindiodeSimHit into datastore
      int saveSimHit(
        const G4int cellId,
        const G4int trackID,
        const G4int pid,
        const G4double tof,
        const G4double edep,
        G4ThreeVector mom,
        TVector3 WightedPos
      );
    protected:
      /** Step processing method
       * @param step the G4Step with the current step information
       * @return true if a Hit has been created, false if the hit was ignored
       */
      bool step(G4Step* step, G4TouchableHistory*);

    private:

      /** members of  SensitiveDetector  */
      //G4double m_thresholdEnergyDeposit;/** Energy Deposit  threshold  */
      //G4double m_thresholdKineticEnergy;/** Kinetic Energy  threshold  */
      int m_hitNum;                     /** The current number of created hits in an event. Used to fill the DataStore ECLHit.*/
      int m_EvnetNumber;                /**  The current number of created hits in an event. Used to fill the DataStore ECL EB array. */
      int m_oldEvnetNumber;                /**  The current number of created hits in an event. Used to fill the DataStore  */
      int m_trackID;                    /** track id */
      G4ThreeVector m_startPos;         /**  Position of prestep */
      G4ThreeVector m_endPos;           /**  Position of poststep*/
      TVector3 m_WightedPos;            /**  Wighted step Position*/
      G4ThreeVector m_momentum;         /**  momentum of track */
      double m_startTime;               /** global time */
      double m_endTime;                 /** global time */
      double m_WightedTime;             /** global time */
      double m_startEnergy;       /** particle energy at the entrance in volume */
      double m_energyDeposit;     /** energy deposited in volume */
      double m_trackLength;       /** length of the track in the volume */

      //int ECLHitIndex[8736][80]; /** Hit index of StoreArray */
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
    };

  } //pindiode namespace
} //Belle2 namespace

#endif
