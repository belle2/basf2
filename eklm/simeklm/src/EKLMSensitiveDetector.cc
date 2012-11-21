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
#include <eklm/geoeklm/EKLMObjectNumbers.h>

#include "G4Step.hh"

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>


namespace Belle2 {

  using namespace EKLM;

  EKLMSensitiveDetector::EKLMSensitiveDetector(G4String name)
    : Simulation::SensitiveDetectorBase(name, KLM)
  {
    GearDir gd = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
    m_mode = (enum EKLMDetectorMode)gd.getInt("Mode");
    gd.append("/SensitiveDetector");
    m_ThresholdEnergyDeposit =
      Unit::convertValue(gd.getDouble("EnergyDepositionThreshold"), "MeV");
    m_ThresholdHitTime =
      Unit::convertValue(gd.getDouble("HitTimeThreshold") , "ns");

    StoreArray<EKLMStepHit> stepHits;
    StoreArray<MCParticle> particles;
    RelationArray particleToStepHits(particles, stepHits);
    registerMCParticleRelation(particleToStepHits);

    StoreArray<EKLMStepHit>::registerPersistent();
    RelationArray::registerPersistent<MCParticle, EKLMStepHit>();
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool EKLMSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
  {

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
    if (eDep <= m_ThresholdEnergyDeposit && m_mode == EKLM_DETECTOR_NORMAL)
      return false;

    /**
     * get reference to the track
     */
    const G4Track& track = * aStep->GetTrack();

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
      B2INFO("EKLMSensitiveDetector: "
             " ALL HITS WITH TIME > hitTimeThreshold ARE DROPPED!!");
      return false;
    }

    /**
     * get reference to a physical volume
     */
    const G4VPhysicalVolume* pv = aStep->GetPreStepPoint()->GetPhysicalVolume();


    /**
     * Get particle information
     */
    const G4int PDGcode = track.GetDefinition()->GetPDGEncoding();


    /**
     * Get Hit position
     */
    const G4ThreeVector& gpos = 0.5 *
                                (aStep->GetPostStepPoint()->GetPosition() +
                                 aStep->GetPreStepPoint()->GetPosition());

    /**
     * Global -> Local position
     */
    const G4ThreeVector& lpos = aStep->GetPostStepPoint()->
                                GetTouchableHandle()->GetHistory()->
                                GetTopTransform().TransformPoint(gpos);

    /**
     * no conversion btw. G4ThreeVector and TVector3 Sad but true
     * GEANT returns in mm!
     * convert to standard units (cm)
     */
    const TVector3  gposRoot = TVector3(Unit::convertValue(gpos.x(), "mm"),
                                        Unit::convertValue(gpos.y(), "mm"),
                                        Unit::convertValue(gpos.z(), "mm"));

    /**
     * no conversion btw. G4ThreeVector and TVector3 Sad but true
     * GEANT returns in mm!
     * convert to standard units(cm)
     */
    const TVector3  lposRoot = TVector3(Unit::convertValue(lpos.x(), "mm"),
                                        Unit::convertValue(lpos.y(), "mm"),
                                        Unit::convertValue(lpos.z(), "mm"));

    /**
     * Get Momentum of the particle
     */
    const G4ThreeVector& momentum = track.GetMomentum();

    /**
     * Get Kinetic energy of the particle
     */
    const double E = track.GetKineticEnergy();

    /**
     * no conversion btw. G4ThreeVector and TVector3 Sad but true
     */
    const TVector3  momentumRoot = TVector3(momentum.x(), momentum.y(), momentum.z());

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
    StoreArray<EKLMStepHit> stepHits;
    int hitNumber = stepHits->GetLast() + 1;
    /*
     * Memory allocation is performed by stepHits->AddrAt(), if necessary.
     * If it fails, this function would not return.
     * No further check is needed.
     */
    EKLMStepHit* hit = new(stepHits->AddrAt(hitNumber))
    EKLMStepHit(momentumRoot, E, trackID, paretntTrackID);
    hit->setLocalPosition(&lposRoot);
    hit->setPosition(&gposRoot);
    hit->setEDep(eDep);
    hit->setPDG(PDGcode);
    hit->setTime(hitTime);
    hit->setEnergy(Ekin);

    /**
     * Get information on mother volumes and store them to the hit.
     */

    const G4PVPlacementGT* pvgt = (G4PVPlacementGT*)pv;

    hit->setVolumeType(pvgt->getVolumeType());
    switch (pvgt->getVolumeType()) {
      case EKLM_SENSITIVE_STRIP:
        hit->setVolumeID(pvgt->getID());
        pvgt = pvgt->getMother();  // Strip
        pvgt = pvgt->getMother();  // StripVolume
        hit->setStrip(stripLocalNumber(pvgt->getID()));
        pvgt = pvgt->getMother();  // Plane
        hit->setPlane(planeLocalNumber(pvgt->getID()));
        break;
      case EKLM_SENSITIVE_SIPM:
        hit->setVolumeID(100000 + pvgt->getID());
        pvgt = pvgt->getMother();  // StripVolume
        hit->setStrip(stripLocalNumber(pvgt->getID()));
        pvgt = pvgt->getMother();  // Plane
        hit->setPlane(planeLocalNumber(pvgt->getID()));
        break;
      case EKLM_SENSITIVE_BOARD:
        hit->setVolumeID(200000 + pvgt->getID());
        pvgt = pvgt->getMother();  // SectionReadoutBoard
        hit->setStrip(-1);
        hit->setPlane(-1);
        break;
      default:
        B2ERROR("EKLMSensitiveDetector.cc:: Try to get hit information from insensitive volumes");
    }
    pvgt = pvgt->getMother(); // Sector
    hit->setSector(sectorLocalNumber(pvgt->getID())); // Sector ID

    pvgt = pvgt->getMother();
    hit->setLayer(layerLocalNumber(pvgt->getID()));  // Layer ID

    pvgt = pvgt->getMother();
    hit->setEndcap(pvgt->getID()); // Endcap ID

    StoreArray<MCParticle> particles;
    RelationArray particleToStepHits(particles, stepHits);
    particleToStepHits.add(track.GetTrackID(), hitNumber);

    return true;
  }

} //namespace Belle II

