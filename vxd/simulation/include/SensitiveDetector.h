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
#include <simulation/kernel/UserInfo.h>

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
     * - SimHits which correspond to Geant4 steps inside the sensitive volume.
     *   There will be several SimHits per traversal, since the step length of
     *   Geant4 is limited to provide detailed information about energy
     *   deposition.
     * - TrueHits are aggregated objects which store the position where a
     *   particle crossed the detector plane (local z=0). There can be only one
     *   TrueHit per traversal of one sensor, but there may be more than one
     *   TrueHit for curling Tracks. TrueHits also carry information about the
     *   particle momenta before and after entering the silicon (or at start/end
     *   point if the track started/ended inside the silicon.
     *   MODIFIED July 2013 (based largely on suggestions by Martin Ritter):
     *   1. Every particle that either enters or leaves the sensitive volume
     *   and deposits some energy there, will create a TrueHit.
     *   2. The (u,v,w) position of the TrueHit are the coordinates of the
     *   midpoint of the track inside the sensitive volume (rather of the
     *   x-ing point with z=0): the w (z) coordinate is added to TrueHits.
     *   3. Each MC particle entering the sensitive volume will be un-ignored,
     *   so that the TrueHits created by secondaries entering the sensitive
     *   volume will be attributed to the correct MCParticle (that is, not to
     *   its primary ancestor).
     *   4. SimHit relations to MCParticles that don't produce a TrueHit and get
     *   re-attributed (to their primary ancestor) will have negative sign. This
     *   concerns the secondaries created inside the sensitive volume that don't
     *   reach another sensitive volume.
     *
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
      SensitiveDetector(VXD::SensorInfoBase* sensorInfo, bool seeNeutrons = false, bool onlyPrimaryTrueHits = false, double sensitiveThreshold = 1.0);

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
      void addTrueHit(G4Step* step, int trackID, int index, double energy,
                      const TVector3& posIn, const TVector3& posOut, const TVector3& momIn, const TVector3& momOut);

      /** Save the collected TrueHit information
       * Called if the TrackID changes or the track leaves the sensitive volume
       */
      void saveTrueHit();

      /** See all particles (irrespective of charge, i.e., in particular, neutrons) */
      bool m_seeNeutrons;
      /** Only create TrueHits for primary particles if true */
      bool m_onlyPrimaryTrueHits;
      /** Threshold on deposited energy per step - discard step if less */
      double m_sensitiveThreshold;
      /** Has this track entered or left the sensitive volume? */
      bool m_trackEnteredOrLeft;
      /** List of all SimHit indices and their energy deposition belonging to the current volume traversal */
      std::vector<std::pair<unsigned int, float> > m_trueHitSteps;
      /** TrackID of the current track. */
      int      m_trueHitTrackID;
      /** Accumulated energy of the current track. */
      double   m_trueHitWeight;
      /** Time when the track reached its mid position. */
      double   m_trueHitTime;
      /** Position of the midpoint of the track. */
      TVector3 m_trueHitPos;
      /** Position at the start of the track. */
      TVector3 m_trueHitPosStart;
      /** Position at the end of the track. */
      TVector3 m_trueHitPosEnd;
      /** Momentum at the midpoint of the track. */
      TVector3 m_trueHitMom;
      /** Momentum at the start of the track. */
      TVector3 m_trueHitMomStart;
      /** Momentum at the end of the track. */
      TVector3 m_trueHitMomEnd;
      /** Cummulative pathlength. */
      std::vector<double> m_trackLengths;
      /** Positions along the track. */
      std::vector<TVector3> m_trackPoints;
      /** Momenta along the track. */
      std::vector<TVector3> m_trackMomenta;
      /** Times along the track. */
      std::vector<double> m_trackTimes;
    };

    template <class SimHitClass, class TrueHitClass>
    SensitiveDetector<SimHitClass, TrueHitClass>::SensitiveDetector(VXD::SensorInfoBase* sensorInfo, bool seeNeutrons, bool onlyPrimaryTrueHits, double sensitiveThreshold):
      VXD::SensitiveDetectorBase(sensorInfo), m_seeNeutrons(seeNeutrons), m_onlyPrimaryTrueHits(onlyPrimaryTrueHits),
      m_sensitiveThreshold(sensitiveThreshold), m_trackEnteredOrLeft(false),
      m_trueHitTrackID(0), m_trueHitWeight(0.0), m_trueHitTime(0.0)
    {
      //Register output collections.
      // Note that we have many SensitiveDetector classes, so this will be attempted many times,
      // therefore we have to suppress errors.
      StoreArray<SimHitClass>::registerPersistent("", DataStore::c_Event, false);
      StoreArray<TrueHitClass>::registerPersistent("", DataStore::c_Event, false);
      RelationArray::registerPersistent<MCParticle, SimHitClass>("", "", DataStore::c_Event, false);
      RelationArray::registerPersistent<MCParticle, TrueHitClass>("", "", DataStore::c_Event, false);
      RelationArray::registerPersistent<TrueHitClass, SimHitClass>("", "", DataStore::c_Event, false);

      // Register MCParticle relations
      const std::string& relMCSimHitsName = DataStore::relationName(
                                              DataStore::arrayName<MCParticle>(""),
                                              DataStore::arrayName<SimHitClass>("")
                                            );
      registerMCParticleRelation(relMCSimHitsName.c_str());
      const std::string& relMCTrueHitsName = DataStore::relationName(
                                               DataStore::arrayName<MCParticle>(""),
                                               DataStore::arrayName<TrueHitClass>("")
                                             );
      registerMCParticleRelation(relMCTrueHitsName.c_str(), RelationArray::c_negativeWeight);
    }

    template <class SimHitClass, class TrueHitClass>
    bool SensitiveDetector<SimHitClass, TrueHitClass>::step(G4Step* step, G4TouchableHistory*)
    {
      // DataStore arrays must be in place even if we don't save anything.
      StoreArray<MCParticle>  particles;
      StoreArray<SimHitClass> simhits;
      if (!simhits.isValid()) simhits.create();
      StoreArray<TrueHitClass> truehits;
      if (!truehits.isValid()) truehits.create();
      RelationArray relMCSimHit(particles, simhits);
      RelationArray relMCTrueHit(particles, truehits);

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
      addTrueHit(step, trackID, hitIndex, depEnergy, posIn, posOut, momIn, momOut);
      //Return true if a SimHit was created
      return hitIndex >= 0;
    }

    template <class SimHitClass, class TrueHitClass>
    int SensitiveDetector<SimHitClass, TrueHitClass>::addSimHit(
      int trackID, int pdg, double time, double energy,
      const TVector3& posIn, const TVector3& posOut, const TVector3& momIn)
    {

      //Ignore all Steps with less than threshold energy deposition
      if (fabs(energy) < m_sensitiveThreshold) return -1;

      VxdID sensorID = m_info->getID();
      const double theta = (posOut - posIn).Theta() * Unit::rad;

      //Create new SimHit.
      StoreArray<MCParticle>  mcParticles;
      StoreArray<SimHitClass> simHits;
      if (!simHits.isValid()) simHits.create();
      RelationArray relMCSimHit(mcParticles, simHits);

      const int hitIndex = simHits.getEntries();
      simHits.appendNew(
        SimHitClass(sensorID, pdg, theta, energy, time, posIn, posOut, momIn)
      );

      //Add relation between the MCParticle and the hit.
      //The index of the MCParticle has to be set to the TrackID and will be
      //replaced later by the correct MCParticle index automatically.
      relMCSimHit.add(trackID, hitIndex);
      return hitIndex;
    }

    template <class SimHitClass, class TrueHitClass>
    void SensitiveDetector<SimHitClass, TrueHitClass>::addTrueHit(
      G4Step* step, int trackID, int index, double energy,
      const TVector3& posIn, const TVector3& posOut, const TVector3& momIn, const TVector3& momOut)
    {
      const G4Track& track = *step->GetTrack();

      //Only create trueHits for primary particles if requested
      if (m_onlyPrimaryTrueHits) {
        const G4PrimaryParticle* primaryParticle = track.GetDynamicParticle()->GetPrimaryParticle();
        if (primaryParticle == NULL) return;
      }

      const G4StepPoint& preStep = *step->GetPreStepPoint();
      const G4StepPoint& posStep = *step->GetPostStepPoint();

      //Check if the step still belongs to the same crossing of the sensitive volume
      //by comparing the trackID. We check for particles leaving the volume
      //after adding the step.
      if (trackID != m_trueHitTrackID) {
        saveTrueHit(); // If we saved before, the data are cleared so nothing happens.
        m_trueHitTrackID = trackID;
        m_trackLengths.push_back(0.0);
        m_trackMomenta.push_back(momIn);
        m_trackPoints.push_back(posIn);
        m_trackTimes.push_back(preStep.GetGlobalTime());
        // Set the flag depending on whether the track entered the volume
        if (preStep.GetStepStatus() == fGeomBoundary) {
          m_trackEnteredOrLeft = true;
          // Unignore the MCParticle iff the track enters the sensitive volume.
          Simulation::TrackInfo::getInfo(track).setIgnore(false);
        } else
          m_trackEnteredOrLeft = false;
      }

      //Remember the index and energy deposit of the SimHit
      if (index >= 0 && energy > 0) m_trueHitSteps.push_back(std::make_pair(index, energy));
      //Increase accumulated deposited energy
      m_trueHitWeight += energy;
      //Update the momentum at the end of traversing the volume
      m_trackMomenta.push_back(momOut);
      m_trackPoints.push_back(posOut);
      //Update accumulators:
      m_trackLengths.push_back(m_trackLengths.back() + step->GetStepLength());
      m_trackTimes.push_back(posStep.GetGlobalTime());

      // Set flag if we are just leaving the volume
      bool isLeaving = (posStep.GetStepStatus() == fGeomBoundary);
      if (isLeaving) {
        m_trackEnteredOrLeft = true;
      }
      //If this step is to the boundary or track gets killed, save TrueHit.
      // We need to save twice since we may never return from here.
      if (isLeaving || track.GetTrackStatus() >= fStopAndKill) saveTrueHit();
    }


    template <class SimHitClass, class TrueHitClass>
    void SensitiveDetector<SimHitClass, TrueHitClass>::saveTrueHit()
    {
      //We save a TrueHit under the following circumstances:
      // - at least one created SimHit
      // - the track entered or exited the sensitive volume
      //If any of these conditions is not met, just clear
      if ((!m_trueHitSteps.empty() || m_seeNeutrons) && m_trackEnteredOrLeft) {
        //Get SensorID and all collections
        VxdID sensorID = m_info->getID();
        StoreArray<MCParticle>   mcParticles;
        StoreArray<SimHitClass>  simHits;
        StoreArray<TrueHitClass> trueHits;
        if (!trueHits.isValid()) trueHits.create();
        RelationArray relMCTrueHits(mcParticles, trueHits);
        RelationArray relTrueSimHit(trueHits, simHits);

        // Edge- and mid-values
        // Locate track midpoint
        double midLength = 0.5 * m_trackLengths.back();
        int midIndexRight = 0;
        while (m_trackLengths.at(midIndexRight) < midLength) midIndexRight++;
        double weightLeft = (m_trackLengths.at(midIndexRight) - midLength) /
                            (m_trackLengths.at(midIndexRight) - m_trackLengths.at(midIndexRight - 1));
        // Mid-time and mid-momentum
        m_trueHitTime = weightLeft * m_trackTimes.at(midIndexRight - 1) +
                        (1.0 - weightLeft) * m_trackTimes.at(midIndexRight);
        // Mid-position: Use 3rd order Bezier curve to interpolate
        TVector3 P0 = m_trackPoints.at(midIndexRight - 1);
        TVector3 P3 = m_trackPoints.at(midIndexRight);
        double momentumScale =
          1.0 / 3.0 * TVector3(P3 - P0).Mag() / m_trackMomenta.at(midIndexRight - 1).Mag();
        TVector3 P1 = P0 + momentumScale * m_trackMomenta.at(midIndexRight - 1);
        TVector3 P2 = P3 - momentumScale * m_trackMomenta.at(midIndexRight);
        // The curve is B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
        m_trueHitPos = (
                         pow(weightLeft, 3) * P0 +
                         3 * pow(weightLeft, 2) * (1 - weightLeft) * P1 +
                         3 * weightLeft * pow(1 - weightLeft, 2) * P2 +
                         pow(1 - weightLeft, 3) * P3
                       );
        // The derivative is dB(t)/dt = 3*[(1-t)^2*(P1-P0)+2*(1-t)*t*(P2-P1)+t^2*(P3-P2)]
        m_trueHitMom = 1.0 / momentumScale * (
                         pow(weightLeft, 2) * (P1 - P0) +
                         2 * weightLeft * (1 - weightLeft) * (P2 - P1) +
                         pow(1 - weightLeft, 2) * (P3 - P2)
                       );
        // We only use first order for momenta:
        //m_trueHitMom = weightLeft * m_trackMomenta[midIndexRight - 1] +
        //               (1 - weightLeft) * m_trackMomenta[midIndexRight];
        m_trueHitPosStart = m_trackPoints.front();
        m_trueHitPosEnd   = m_trackPoints.back();
        m_trueHitMomStart = m_trackMomenta.front();
        m_trueHitMomEnd   = m_trackMomenta.back();
        int hitIndex = trueHits.getEntries();
        trueHits.appendNew(
          TrueHitClass(sensorID,
                       m_trueHitPos.X(), m_trueHitPos.Y(), m_trueHitPos.Z(),
                       m_trueHitPosStart.X(), m_trueHitPosStart.Y(), m_trueHitPosStart.Z(),
                       m_trueHitPosEnd.X(), m_trueHitPosEnd.Y(), m_trueHitPosEnd.Z(),
                       m_trueHitWeight, m_trueHitTime, m_trueHitMom, m_trueHitMomStart,
                       m_trueHitMomEnd)
        );
        //Add Relation to MCParticle
        relMCTrueHits.add(m_trueHitTrackID, hitIndex, m_trueHitWeight);
        //Add Relation to SimHits
        relTrueSimHit.add(hitIndex, m_trueHitSteps.begin(), m_trueHitSteps.end());
      }

      //Clear the collected steps and reset the TrackID
      m_trackEnteredOrLeft = false;
      m_trueHitSteps.clear();
      m_trueHitTrackID = 0;
      m_trueHitWeight  = 0.0;
      m_trueHitTime    = 0.0;
      m_trueHitPos     = TVector3(0, 0, 0);
      m_trueHitMom     = TVector3(0, 0, 0);
      m_trackLengths.clear();
      m_trackPoints.clear();
      m_trackMomenta.clear();
      m_trackTimes.clear();
    }

  } //VXD Namespace

} //Belle2 namespace
#endif
