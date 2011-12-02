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

#include <framework/logging/Logger.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>

#include "G4Step.hh"


#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>


namespace Belle2 {


  EKLMSensitiveDetector::EKLMSensitiveDetector(G4String name)
      : Simulation::SensitiveDetectorBase(name, KLM)
  {
    GearDir gd = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
    m_mode = gd.getInt("Mode");
    gd.append("/SensitiveDetector");
    m_ThresholdEnergyDeposit = gd.getDouble("EnergyDepositionThreshold") / MeV;
    m_ThresholdHitTime = gd.getDouble("HitTimeThreshold") / ns;
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool EKLMSensitiveDetector::step(G4Step *aStep, G4TouchableHistory *)
  {
    std::cout << "EKLM SD called" << std::endl;
    /**
     * Get deposited energy
     */
    const G4double eDep = aStep->GetTotalEnergyDeposit();

    /**
     * in normal opearation mode (m_mode=0)
     * ignore tracks with small energy deposition
     * use "<=" instead of "<" to drop hits from neutrinos etc unless eDepositionThreshold is non-negative
     * Background studies m_mode=1 accepts all tracks
     */
    if (eDep <= m_ThresholdEnergyDeposit && m_mode == 0)
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
    * drop hit if global time is nan or if it is  more than
    * hitTimeThreshold (to avoid nuclei fission signals)
    */
    if (isnan(hitTime)) {
      B2ERROR("EKLMSensitiveDetector: global time is nan");
      return false;
    }
    // No time cut for background studeis
    if (hitTime > m_ThresholdHitTime && m_mode == 0) {
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


    std::cout << "EKLM SD creates a hit in " << ((G4PVPlacementGT*)pv)->getName() << " volume" << std::endl;

    /**
     * creates step hit and store in to DataStore
     */
    EKLMStepHit *hit = new(m_stepHitsArray->AddrAt(m_stepHitsArray.getEntries()))
    EKLMStepHit(PDGcode, hitTime, Ekin, gposRoot, momentumRoot, eDep, trackID, paretntTrackID, pv);
    if (hit == NULL) {
      B2ERROR("EKLMSensitiveDetector.cc:: Memory allocation error. Cannot allocate hit in stepHitsArray");
      return false;
    }

    /**
     * Get information on mother volumes and store them to the hit
     */
    const G4PVPlacementGT *pvgt = (G4PVPlacementGT*)pv;
    hit->setVolumeType(pvgt->getVolumeType());
    if (pvgt->getVolumeType() >= 0) {
      pvgt = pvgt->getMother();

      if (hit->getVolumeType() == 0) { // Sensitive strip
        pvgt = pvgt->getMother();
      }
      hit->setStrip(pvgt->getID()); // StripVolume ID for Strip and SiPM and BaseBoard ID for StripBoard

      if (hit->getVolumeType() == 0 || hit->getVolumeType() == 1) { // Sensitive strip
        pvgt = pvgt->getMother();
      }
      hit->setPlane(pvgt->getID());  // Plane ID for Stripand and SiPM and SectionBoard ID for StripBoard

      pvgt = pvgt->getMother();
      hit->setSector(pvgt->getID()); // Sector ID

      pvgt = pvgt->getMother();
      hit->setLayer(pvgt->getID());  // Layer ID

      pvgt = pvgt->getMother();
      hit->setEndcap(pvgt->getID()); // Endcap ID
    } else
      B2ERROR("EKLMSensitiveDetector.cc:: Try to get hit information from insensitive volumes");




    //     StoreArray<MCParticle> MCParticlesArray;
    //     RelationArray particleToSimHitsRelation(MCParticlesArray, simHitsArray);
    //     registerMCParticleRelation(particleToSimHitsRelation);
    //     particleToSimHitsRelation.add(track.GetTrackID(),
    //                                   simHitsArray.getEntries());

    return true;
  }

} //namespace Belle II

