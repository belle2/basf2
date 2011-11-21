/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <eklm/simeklm/EKLMSensitiveDetector.h>
#include <eklm/dataobjects/EKLMStepHit.h>

#include <framework/logging/Logger.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>

#include "G4Step.hh"
//#include "G4SteppingManager.hh"


#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>


namespace Belle2 {


  EKLMSensitiveDetector::EKLMSensitiveDetector(G4String name)
      : Simulation::SensitiveDetectorBase(name, KLM)
  {
    GearDir SensitiveDetector = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/SensitiveDetector");
    m_ThresholdEnergyDeposit = SensitiveDetector.getDouble("EnergyDepositionThreshold") / MeV;
    m_ThresholdHitTime = SensitiveDetector.getDouble("HitTimeThreshold") / ns;
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool EKLMSensitiveDetector::step(G4Step *aStep, G4TouchableHistory *)
  {

    /**
     * Get deposited energy
     */
    const G4double eDep = aStep->GetTotalEnergyDeposit();

    /**
     * ignore tracks with small energy deposition
     * use "<=" instead of "<" to drop hits from neutrinos etc unless eDepositionThreshold is non-negative
     */
    if (eDep <= m_ThresholdEnergyDeposit)
      return false;

    /**
     * get reference to the track
     */
    const G4Track & track = * aStep->GetTrack();

    /**
     * get time of hit
     */
    const G4double hitTime = track.GetGlobalTime();


    /**
    * drop hit if global time is nan or if it is  mothe than
    * hitTimeThreshold (to avoid nuclei fission signals)
    */
    if (isnan(hitTime)) {
      B2ERROR("EKLMSensitiveDetector: global time is nan");
      return false;
    }
    if (hitTime > m_ThresholdHitTime) {
      B2INFO("EKLMSensitiveDetector: ALL HITS WITH TIME > hitTimeThreshold ARE DROPPED!!");
      return false;
    }

    /**
     * get reference to a physical volume
     */
    const G4VPhysicalVolume *pv = aStep->GetPreStepPoint()->GetPhysicalVolume();


    /**
     * Get particle information
     */
    const G4int PDGcode = track.GetDefinition()->GetPDGEncoding();


    /**
     * Get Hit position
     */
    const G4ThreeVector & gpos = 0.5 *
                                 (aStep->GetPostStepPoint()->GetPosition() +
                                  aStep->GetPreStepPoint()->GetPosition());

    /**
     * no conversion btw. G4ThreeVector and TVector3 Sad but true
     */
    const TVector3 & gposRoot = TVector3(gpos.x(), gpos.y(), gpos.z());

    /**
     * Get Momentum of the particle
     */
    const G4ThreeVector & momentum = track.GetMomentum();
    /**
     * no conversion btw. G4ThreeVector and TVector3 Sad but true
     */
    const TVector3 & momentumRoot = TVector3(momentum.x(), momentum.y(), momentum.z());

    /**
     * Get Kinetic energy of the particle
     */
    const double Ekin = track.GetKineticEnergy();

    /**
     * get  track ID
     */
    const int trackID = track.GetTrackID();

    /**
     * get parent track ID
     */
    const int paretntTrackID = track.GetParentID();


    /**
     * creates step hit and store in to DataStore
     */
    StoreArray<EKLMStepHit> stepHitsArray;
    EKLMStepHit *hit = new(stepHitsArray->AddrAt(stepHitsArray.getEntries()))
    EKLMStepHit(PDGcode, hitTime, Ekin, gposRoot, momentumRoot, eDep, trackID, paretntTrackID, pv);
    if (hit == NULL) {
      B2ERROR("EKLMSensitiveDetector.cc:: Memory allocation error. Cannot allocate hit in stepHitsArray");
      return false;
    }

    /**
     * Get information on mother volumes and store them to the hit
     */
    G4PVPlacementGT *pvgt = (G4PVPlacementGT*)pv;
    pvgt = pvgt->getMother();
    pvgt = pvgt->getMother();
    hit->setStrip(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->setPlane(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->setSector(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->setLayer(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->setEndcap(pvgt->getID());




    //     StoreArray<MCParticle> MCParticlesArray;
    //     RelationArray particleToSimHitsRelation(MCParticlesArray, simHitsArray);
    //     registerMCParticleRelation(particleToSimHitsRelation);
    //     particleToSimHitsRelation.add(track.GetTrackID(),
    //                                   simHitsArray.getEntries());

    return true;
  }

} //namespace Belle II

