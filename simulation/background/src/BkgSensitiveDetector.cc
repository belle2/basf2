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
    Simulation::SensitiveDetectorBase("BKG", Const::invalidDetector), m_trackID(0), m_startPos(0., 0., 0.), m_startMom(0., 0., 0.),
    m_startTime(0), m_startEnergy(0), m_energyDeposit(0), m_trackLength(0.)
  {
    // registration of store arrays and relations

    StoreArray<MCParticle> mcParticles;
    StoreArray<BeamBackHit> beamBackHits;

    beamBackHits.registerInDataStore();
    mcParticles.registerRelationTo(beamBackHits);

    // additional registration of MCParticle relation

    RelationArray  relation(mcParticles, beamBackHits);
    registerMCParticleRelation(relation);

    m_eclrepscale = 0;
    std::string subDet = subDett;
    if (subDet == "IR") m_subDet = 0;
    else if (subDet ==   "PXD") m_subDet = 1;
    else if (subDet ==   "SVD") m_subDet = 2;
    else if (subDet ==   "CDC") m_subDet = 3;
    else if (subDet == "ARICH") m_subDet = 4;
    else if (subDet ==   "TOP") m_subDet = 5;
    else if (subDet ==   "ECL") m_subDet = 6;
    else if (subDet ==  "EKLM") m_subDet = 7;
    else if (subDet ==  "BKLM") m_subDet = 8;
    else m_subDet = 99;

    if (m_subDet == 6) {
      const int nc[] = {3, 4, 6, 9, 2, 9, 6, 4}; // the number of crystals in a sector per ring
      const int indx[] = {96, 288, 864, 1151, 7776, 8064, 8544, 8736}; // regions with the same number of crystals in a ring
      std::vector<int> indxv(indx, indx + sizeof(indx) / sizeof(int));
      m_eclrepscale = nc[upper_bound(indxv.begin(), indxv.end(), iden) - indxv.begin()];
    }

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
      int id = m_identifier;
      if (m_subDet == 6) { // ECL
        int sector = preStep.GetTouchable()->GetCopyNumber(1);
        if ((sector == 0) && (m_eclrepscale == 2) && ((id & 1) != 0)) id += 144; // barrel odd-numbered diodes in sector 0 -> sector 144
        id += sector * m_eclrepscale;
      }
      beamBackHits.appendNew(m_subDet, id, pdgCode, m_trackID, m_startPos, m_startMom, m_startTime, m_startEnergy, endEnergy,
                             m_energyDeposit, m_trackLength, neutWeight);


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
