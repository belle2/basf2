/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj and Marko Petric                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/background/BkgSensitiveDetector.h>
#include <simulation/background/BkgNeutronWeight.h>
#include <simulation/dataobjects/BeamBackHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>
#include <G4TrackingManager.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {


  BkgSensitiveDetector::BkgSensitiveDetector(const char* subDett, int iden):
    Simulation::SensitiveDetectorBase("BKG", Const::invalidDetector), m_trackID(0), m_startPos(0., 0., 0.), m_startMom(0., 0., 0.), m_startTime(0), m_startEnergy(0), m_energyDeposit(0), m_trackLength(0.)
  {
    StoreArray<MCParticle> mcParticles;
    StoreArray<BeamBackHit> beamBackHits;

    RelationArray  relBeamBackHitToMCParticle(mcParticles, beamBackHits);
    registerMCParticleRelation(relBeamBackHitToMCParticle);

    beamBackHits.registerAsPersistent();
    relBeamBackHitToMCParticle.registerAsPersistent();

    std::string subDet = subDett;
    if (subDet == "IR")    m_subDet = 0;
    else if (subDet == "PXD")   m_subDet = 1;
    else if (subDet == "SVD")   m_subDet = 2;
    else if (subDet == "CDC")   m_subDet = 3;
    else if (subDet == "ARICH")   m_subDet = 4;
    else if (subDet == "TOP") m_subDet = 5;
    else if (subDet == "ECL")   m_subDet = 6;
    else if (subDet == "EKLM")  m_subDet = 7;
    else if (subDet == "BKLM")  m_subDet = 8;
    else                        m_subDet = 99;

    m_identifier = iden;

  }


  G4bool BkgSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
  {

    const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
    G4Track& track  = *aStep->GetTrack();

    if (m_trackID != track.GetTrackID()) {
      //TrackID changed, store track informations
      m_trackID = track.GetTrackID();
      //Get world position
      const G4ThreeVector& worldPosition = preStep.GetPosition();
      m_startPos.SetXYZ(worldPosition.x() * Unit::mm / Unit::cm , worldPosition.y() * Unit::mm / Unit::cm,
                        worldPosition.z() * Unit::mm / Unit::cm);
      //Get momentum
      const G4ThreeVector& momentum = preStep.GetMomentum() ;
      m_startMom.SetXYZ(momentum.x() * Unit::MeV, momentum.y() * Unit::MeV ,
                        momentum.z() * Unit::MeV);
      //Get time
      m_startTime = preStep.GetGlobalTime();
      //Get energy
      m_startEnergy =  preStep.GetKineticEnergy() * Unit::MeV;
      //Reset energy deposit;
      m_energyDeposit = 0;
      //Reset track lenght;
      m_trackLength = 0;
    }
    //Update energy deposit
    m_energyDeposit += aStep->GetTotalEnergyDeposit() * Unit::MeV;

    m_trackLength += aStep->GetStepLength() * Unit::mm;

    //Save Hit if track leaves volume or is killed
    if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
      int pdgCode = track.GetDefinition()->GetPDGEncoding();
      double endEnergy = track.GetKineticEnergy() * Unit::MeV;
      double neutWeight = 0;
      if (pdgCode == 2112) {
        BkgNeutronWeight& wt = BkgNeutronWeight::getInstance();
        neutWeight = wt.getWeight(m_startEnergy / Unit::MeV);
      }
      StoreArray<BeamBackHit> beamBackHits;
      int nentr = beamBackHits.getEntries();
      new(beamBackHits.nextFreeAddress()) BeamBackHit(m_subDet, m_identifier, pdgCode, m_trackID, m_startPos, m_startMom, m_startTime, endEnergy, m_startEnergy, m_energyDeposit, m_trackLength, neutWeight);


      // create relation to MCParticle
      StoreArray<MCParticle> mcParticles;
      RelationArray relBeamBackHitToMCParticle(mcParticles, beamBackHits);
      relBeamBackHitToMCParticle.add(m_trackID, nentr);

      //Reset TrackID
      m_trackID = 0;
    }

    return true;
  }


} // end of namespace Belle2
