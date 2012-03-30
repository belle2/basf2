/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka             *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_SENSITIVEDETECTOR_H
#define VXD_SENSITIVEDETECTOR_H

#include <vxd/simulation/SensitiveDetectorBase.h>
#include <simulation/dataobjects/BeamBackHit.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>

// Geant4
#include <G4Types.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {
    /** Sensitive Detector implementation of PXD and SVD.
     * This class provides the actual implementation of the hit generation for
     * PXD and SVD. It is templated to be able to create the corresponding
     * output collection.
     *
     * It generates two different kinds of Hits:
     * - SimHits which correspond to Geant4 Energy depositions. There can and
     *   will most probably be more than one SimHit per traversal since the
     *   Steplength of Geant4 is limited to provide detailed information about
     *   the energy deposition
     * - TrueHits are aggregated objects which store the position a particle
     *   crossed the detector plane (local z=0). There can be only one TrueHit
     *   per traversal of one sensor, but there may be more than one TrueHit
     *   for curling Tracks. TrueHits also carry information about the particle
     *   momentum before and after entering the silicon (or at start/end point
     *   if the track started/ended inside the silicon.
     *
     * @tparam SimHitClass Class to use when generating SimHits
     * @tparam TrueHitClass Class to use when generating TrueHits
     */
    template<class SimHitClass, class TrueHitClass> class SensitiveDetector: public SensitiveDetectorBase {
    public:
      /** Constructor
       * @param info pointer to a valid SensorInfo instance providing
       * information about the sensor this sensitive detector instance will
       * handle. Ownership of the SensorInfo goes to the sensitive detector
       * instance
       */
      SensitiveDetector(VXD::SensorInfoBase* sensorInfo, bool seeNeutrons = false, bool onlyPrimaryTrueHits = false);

    protected:
      /** Process one step inside the sensitive volume.
       * @param step Step Information
       * @return true if a hit was created, false otherwise
       */
      bool step(G4Step* step, G4TouchableHistory*);

      /** Add a SimHit to the appropriate collection
       * @param step    Step Information
       * @param trackID Geant4 TrackID
       * @param pdg     PDG Particle Code of the particle
       * @param time    Timestamp of the step
       * @param energy  Deposited energy
       * @param posIn   Start point of the Step
       * @param posOut  End point of the Step
       * @param momIn   Momentum at beginning of the Step
       * @param momOut  Momentum at end of the Step
       */
      int addSimHit(int trackID, int pdg, double time, double energy,
                    const TVector3& posIn, const TVector3& posOut, const TVector3& momIn);

      /** Update TrueHit information and create TrueHit if appropriate.
       * All SimHits of a single traversal of the volume will be collected.
       * If the particle traverses the detector plane (local z=0), a TrueHit
       * with the coordinates of this traversal will be created. If the
       * particle traverses the plane more than once while staying inside the
       * sensitive volume (curling), all crossings will be averaged.
       *
       * @param step    Step Information
       * @param trackID Geant4 TrackID
       * @param index   SimHit index of the Simhit created by the same step, -1 if no simhit was created
       * @param time    Timestamp of the step
       * @param energy  Deposited energy
       * @param posIn   Start point of the Step
       * @param posOut  End point of the Step
       * @param momIn   Momentum at beginning of the Step
       * @param momOut  Momentum at end of the Step
       */
      void addTrueHit(G4Step* step, int trackID, int index, double time, double energy,
                      const TVector3& posIn, const TVector3& posOut, const TVector3& momIn, const TVector3& momOut);

      /** Save the collected TrueHit information
       * Called if the TrackID changes or the track leaves the sensitive volume
       */
      void saveTrueHit();

      /** See all particles (irrespective of charge, i.e., in particular, neutrons) */
      bool m_seeNeutrons;
      /** Only create TrueHits for primary particles if true */
      bool m_onlyPrimaryTrueHits;
      /** List of all SimHit indices and their energy deposition belonging to the current volume traversal */
      std::vector<std::pair<unsigned int, float> > m_trueHitSteps;
      /** TrackID of the current volume traversal */
      int      m_trueHitTrackID;
      /** Number of crossings of the detector plane (z=0) for the current volume traversal */
      int      m_trueHitCount;
      /** Accumulated energy of the current volume traversal */
      double   m_trueHitWeight;
      /** Timestamp for crossing the detector plane (z=0) of the current volume traversal */
      double   m_trueHitTime;
      /** Position of the crossing of the detector plane (z=0) for the current volume traversal */
      TVector3 m_trueHitPos;
      /** Position at the begin of the current volume traversal */
      TVector3 m_trueHitPosStart;
      /** Position at the end of the current volume traversal */
      TVector3 m_trueHitPosEnd;
      /** Momentum of the crossing of the detector plane (z=0) for the current volume traversal */
      TVector3 m_trueHitMom;
      /** Momentum at the begin of the current volume traversal */
      TVector3 m_trueHitMomStart;
      /** Momentum at the end of the current volume traversal */
      TVector3 m_trueHitMomEnd;
    };

    template <class SimHitClass, class TrueHitClass>
    SensitiveDetector<SimHitClass, TrueHitClass>::SensitiveDetector(VXD::SensorInfoBase* sensorInfo, bool seeNeutrons, bool onlyPrimaryTrueHits):
      VXD::SensitiveDetectorBase(sensorInfo), m_seeNeutrons(seeNeutrons), m_onlyPrimaryTrueHits(onlyPrimaryTrueHits),
      m_trueHitTrackID(0), m_trueHitCount(0), m_trueHitWeight(0.0), m_trueHitTime(0.0)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<SimHitClass>  simHits;
      StoreArray<TrueHitClass> trueHits;
      RelationArray relMCSimHit(mcParticles, simHits);
      RelationArray relMCTrueHit(mcParticles, trueHits);
      RelationArray relTrueSimHit(trueHits, simHits);
      registerMCParticleRelation(relMCSimHit);
      registerMCParticleRelation(relMCTrueHit);
    }

    template <class SimHitClass, class TrueHitClass>
    bool SensitiveDetector<SimHitClass, TrueHitClass>::step(G4Step* step, G4TouchableHistory*)
    {
      // Get track
      const G4Track& track    = *step->GetTrack();
      // Get particle PDG code
      const int pdgCode       = track.GetDefinition()->GetPDGEncoding();
      // Get particle charge (only keep charged tracks and photons)
      const double minCharge  = 0.01 * Unit::e;
      const double pdgCharge  = track.GetDefinition()->GetPDGCharge() * Unit::e;
      bool isAllowedNeutral = (pdgCode == 22) || (m_seeNeutrons && (abs(pdgCode) == 2112));
      if ((fabs(pdgCharge) < minCharge) && !isAllowedNeutral) return false;
      // Get track ID
      const int trackID       = track.GetTrackID();
      //Get deposited energy
      const double depEnergy  = step->GetTotalEnergyDeposit() * Unit::MeV;
      // Get time of the step
      const double globalTime = track.GetGlobalTime() * Unit::ns ;
      // Get step information
      const G4StepPoint& preStep      = *step->GetPreStepPoint();
      const G4StepPoint& posStep      = *step->GetPostStepPoint();
      const G4AffineTransform& topTransform = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform();

      const G4ThreeVector& preStepPos = topTransform.TransformPoint(preStep.GetPosition()) * Unit::mm;
      const G4ThreeVector& posStepPos = topTransform.TransformPoint(posStep.GetPosition()) * Unit::mm;
      const G4ThreeVector& preStepMom = topTransform.TransformAxis(preStep.GetMomentum()) * Unit::MeV;
      const G4ThreeVector& posStepMom = topTransform.TransformAxis(posStep.GetMomentum()) * Unit::MeV;

      TVector3 posIn(preStepPos.x(), preStepPos.y(), preStepPos.z());
      TVector3 posOut(posStepPos.x(), posStepPos.y(), posStepPos.z());
      TVector3 momIn(preStepMom.x(), preStepMom.y(), preStepMom.z());
      TVector3 momOut(posStepMom.x(), posStepMom.y(), posStepMom.z());

      //Create SimHit
      int hitIndex = addSimHit(trackID, pdgCode, globalTime, depEnergy, posIn, posOut, momIn);
      //Update TrueHit information and create TrueHit if appropriate
      addTrueHit(step, trackID, hitIndex, globalTime, depEnergy, posIn, posOut, momIn, momOut);
      //Return true if a SimHit was created
      return hitIndex >= 0;
    }

    template <class SimHitClass, class TrueHitClass>
    int SensitiveDetector<SimHitClass, TrueHitClass>::addSimHit(
      int trackID, int pdg, double time, double energy,
      const TVector3& posIn, const TVector3& posOut, const TVector3& momIn)
    {

      //Ignore all Steps with less than 1eV Energydeposition for now
      //FIXME: make this more elaborate
      const double thresholdEnergy = 0.01 * Unit::eV;
      if (fabs(energy) < thresholdEnergy) return -1;

      VxdID sensorID = m_info->getID();
      const double theta = (posOut - posIn).Theta() * Unit::rad;

      //Create new SimHit.
      StoreArray<MCParticle>  mcParticles;
      StoreArray<SimHitClass> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      const int hitIndex = simHits->GetLast() + 1 ;
      new(simHits->AddrAt(hitIndex)) SimHitClass(sensorID, pdg, theta, energy, time,
                                                 posIn, posOut, momIn);

      //Add relation between the MCParticle and the hit.
      //The index of the MCParticle has to be set to the TrackID and will be
      //replaced later by the correct MCParticle index automatically.
      relMCSimHit.add(trackID, hitIndex);
      return hitIndex;
    }

    template <class SimHitClass, class TrueHitClass>
    void SensitiveDetector<SimHitClass, TrueHitClass>::addTrueHit(
      G4Step* step, int trackID, int index, double time, double energy,
      const TVector3& posIn, const TVector3& posOut, const TVector3& momIn, const TVector3& momOut)
    {
      const G4Track& track = *step->GetTrack();

      //Only create trueHits for primary particles if requested
      if (m_onlyPrimaryTrueHits) {
        const G4PrimaryParticle* primaryParticle = track.GetDynamicParticle()->GetPrimaryParticle();
        if (primaryParticle == NULL) return;
      }

      //Check if the step still belongs to the same crossing of the sensitive volume
      //by comparing the trackID. We check for particles leaving the volume
      //after adding the step.
      if (trackID != m_trueHitTrackID) {
        saveTrueHit();
        m_trueHitTrackID = trackID;
        m_trueHitMomStart = momIn;
        m_trueHitPosStart = posIn;
      }

      //Remember the index and energy deposit of the SimHit
      if (index >= 0 && energy > 0) m_trueHitSteps.push_back(std::make_pair(index, energy));
      //Increase accumulated deposited energy
      m_trueHitWeight += energy;
      //Update the momentum at the end of traversing the volume
      m_trueHitMomEnd = momOut;
      m_trueHitPosEnd = posOut;
      //Check if step crossed local z=0
      if (posIn.Z()*posOut.Z() < 0) {
        //Linear propagate to z=0
        TVector3 dir = posOut - posIn;
        TVector3 posZero = posIn - posIn.Z() / dir.Z() * dir;
        //Average momentum between pre and post step
        double fraction = (posZero - posIn).Mag() / dir.Mag();
        TVector3 momZero = momIn * fraction + momOut * (1 - fraction);
        if (m_trueHitCount == 0) {
          //If we did not have a crossing yet, simply take the values as we have them
          m_trueHitPos = posZero;
          m_trueHitMom = momZero;
          m_trueHitTime = time;
        } else {
          //Otherwise take the avg of all crossings
          m_trueHitPos  = (m_trueHitCount * m_trueHitPos + posZero) * (1.0 / (m_trueHitCount + 1));
          m_trueHitMom  = (m_trueHitCount * m_trueHitMom + momZero) * (1.0 / (m_trueHitCount + 1));
          m_trueHitTime = (m_trueHitCount * m_trueHitTime + time)   * (1.0 / (m_trueHitCount + 1));
        }
        //Increase number of crossings
        m_trueHitCount++;
      }
      //If this step is in boundary or track gets killed, save TrueHit
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) saveTrueHit();
    }


    template <class SimHitClass, class TrueHitClass>
    void SensitiveDetector<SimHitClass, TrueHitClass>::saveTrueHit()
    {
      //We save a TrueHit under the following circumstances:
      // - at least one created SimHit
      // - at least one crossing of the detector plane, local z=0
      //If any of this conditions is not met, just clear
      if ((!m_trueHitSteps.empty() || m_seeNeutrons) && m_trueHitCount > 0) {
        //Get SensorID and all collections
        VxdID sensorID = m_info->getID();
        StoreArray<MCParticle>   mcParticles;
        StoreArray<SimHitClass>  simHits;
        StoreArray<TrueHitClass> trueHits;
        RelationArray relMCTrueHits(mcParticles, trueHits);
        RelationArray relTrueSimHit(trueHits, simHits);

        //Create a new TrueHit
        int hitIndex = trueHits->GetLast() + 1;
        new(trueHits->AddrAt(hitIndex))
        TrueHitClass(sensorID, m_trueHitPos.X(), m_trueHitPos.Y(), m_trueHitPosStart.X(), m_trueHitPosStart.Y(), m_trueHitPosEnd.X(), m_trueHitPosEnd.Y(), m_trueHitWeight, m_trueHitTime,
                     m_trueHitMom, m_trueHitMomStart, m_trueHitMomEnd);
        //Add Relation to MCParticle
        relMCTrueHits.add(m_trueHitTrackID, hitIndex, m_trueHitWeight);
        //Add Relation to SimHits
        relTrueSimHit.add(hitIndex, m_trueHitSteps.begin(), m_trueHitSteps.end());
      }

      //Clear the collected steps and reset the TrackID
      m_trueHitSteps.clear();
      m_trueHitTrackID = 0;
      m_trueHitCount   = 0;
      m_trueHitWeight  = 0.0;
      m_trueHitTime    = 0.0;
    }

  } //VXD Namespace

} //Belle2 namespace
#endif
