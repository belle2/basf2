/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <ecl/simulation/SensitiveDetector.h>
#include <ecl/geometry/ECLGeometryPar.h>

using namespace std;
using namespace Belle2::ECL;

SensitiveDetector::SensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
  Simulation::SensitiveDetectorBase(name, Const::ECL),
  m_eclSimHitRel(m_mcParticles, m_eclSimHits),
  m_eclHitRel(m_mcParticles, m_eclHits),
  m_thresholdEnergyDeposit(thresholdEnergyDeposit),
  m_thresholdKineticEnergy(thresholdKineticEnergy)
{
  m_oldEvnetNumber = -1;
  m_trackID = 0;
  m_WeightedTime = 0;
  m_energyDeposit = 0;

  registerMCParticleRelation(m_eclSimHitRel);
  registerMCParticleRelation(m_eclHitRel);

  m_eclSimHits.registerInDataStore();
  m_eclHits.registerInDataStore();

  m_mcParticles.registerRelationTo(m_eclSimHits);
  m_mcParticles.registerRelationTo(m_eclHits);
}

SensitiveDetector::~SensitiveDetector()
{
}

void SensitiveDetector::Initialize(G4HCofThisEvent*)
{
}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
{
  G4double edep = aStep->GetTotalEnergyDeposit();
  const G4StepPoint& s0 = *aStep->GetPreStepPoint();
  const G4StepPoint& s1 = *aStep->GetPostStepPoint();
  const G4Track&  track = *aStep->GetTrack();

  if (m_trackID != track.GetTrackID()) { //TrackID changed, store track informations
    m_trackID = track.GetTrackID();
    m_momentum = s0.GetMomentum(); // Get momentum
    //Reset track parameters
    m_energyDeposit = 0;
    m_WeightedTime = 0;
    m_WeightedPos.set(0, 0, 0);
  }
  //Update energy deposit
  G4double hedep = 0.5 * edep; // half of energy deposition -- for averaging purpose
  m_energyDeposit += edep;
  m_WeightedTime  += (s0.GetGlobalTime() + s1.GetGlobalTime()) * hedep;
  m_WeightedPos   += (s0.GetPosition()   + s1.GetPosition()) * hedep;
  //Save Hit if track leaves volume or is killed
  if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
    if (m_energyDeposit > 0.0) {
      ECLGeometryPar* eclp = ECLGeometryPar::Instance();
      G4int cellID = eclp->TouchableToCellID(s0.GetTouchable());
      G4double dTotalEnergy = 1 / m_energyDeposit;
      m_WeightedTime *= dTotalEnergy;
      m_WeightedPos  *= dTotalEnergy;
      int pdgCode = track.GetDefinition()->GetPDGEncoding();
      saveSimHit(cellID, m_trackID, pdgCode, m_WeightedTime, m_energyDeposit, m_momentum, m_WeightedPos);
    }
    //Reset TrackID
    m_trackID = 0;
  }
  return true;
}

void SensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
}

int SensitiveDetector::saveSimHit(G4int cellId, G4int trackID, G4int pid, G4double tof, G4double edep,
                                  const G4ThreeVector& mom, const G4ThreeVector& pos)
{
  int currentEvnetNumber = m_eventMetaDataPtr->getEvent();
  if (currentEvnetNumber != m_oldEvnetNumber) {
    m_oldEvnetNumber = currentEvnetNumber;
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      for (int  TimeIndex = 0; TimeIndex < 80; TimeIndex++) {
        m_ECLHitIndex[iECLCell][TimeIndex] = -1;
      }
    }
  }

  if (!m_eclSimHits) m_eclSimHits.create();
  if (!   m_eclHits)    m_eclHits.create();

  TVector3 momentum(mom.getX(), mom.getY(), mom.getZ());
  TVector3 position(pos.getX(), pos.getY(), pos.getZ());
  momentum *= 1 / CLHEP::GeV;
  position *= 1 / CLHEP::cm;
  tof      *= 1 / CLHEP::ns;
  edep     *= 1 / CLHEP::GeV;
  m_eclSimHits.appendNew(cellId + 1, trackID, pid, tof, edep, momentum, position);

  int simhitNumber = m_eclSimHits.getEntries() - 1;
  m_eclSimHitRel.add(trackID, simhitNumber);

  if (tof < 8000) {
    ECLGeometryPar* eclp = ECLGeometryPar::Instance();
    const TVector3& PosCell = eclp->GetCrystalPos(cellId);        // center of crystal position
    const TVector3& VecCell = eclp->GetCrystalVec(cellId);        // vector of crystal axis
    double z = 15. - (position - PosCell) * VecCell;       // position along the vector of crystal axis
    double tsen = 6.05 + z * (0.0749 - z * 0.00112) + tof; // flight time to diode sensor

    int TimeIndex = tof * (1. / 100);                      // Hit Time of StoreArray
    int hitNum = m_ECLHitIndex[cellId][TimeIndex];
    if (hitNum == -1) {
      hitNum = m_eclHits.getEntries();
      m_eclHitRel.add(trackID, hitNum);
      m_eclHits.appendNew(cellId + 1, edep, tsen);
      m_ECLHitIndex[cellId][TimeIndex] = hitNum;
    } else {
      m_eclHitRel.add(trackID, hitNum);
      ECLHit* eclHit  = m_eclHits[hitNum];
      double old_edep = eclHit->getEnergyDep();
      double old_tsen = eclHit->getTimeAve();
      double new_edep = old_edep + edep;
      eclHit->setEnergyDep(new_edep);
      eclHit->setTimeAve((old_edep * old_tsen + edep * tsen) / new_edep);
    }
  }
  return simhitNumber;
}
