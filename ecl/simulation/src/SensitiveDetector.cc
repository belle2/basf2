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

#define UNUSED(x)

SensitiveDetector::SensitiveDetector(G4String name, G4double UNUSED(thresholdEnergyDeposit),
                                     G4double UNUSED(thresholdKineticEnergy)):
  Simulation::SensitiveDetectorBase(name, Const::ECL),
  m_eclSimHitRel(m_mcParticles, m_eclSimHits),
  m_eclHitRel(m_mcParticles, m_eclHits)//,
  // m_thresholdEnergyDeposit(thresholdEnergyDeposit),
  // m_thresholdKineticEnergy(thresholdKineticEnergy)
{
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
  //  return true;
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
  struct hit_t { double e, t; };
  map<int, hit_t> a;

  struct thit_t { int tid, hid; };
  vector<thit_t> tr;

  ECLGeometryPar* eclp = ECLGeometryPar::Instance();

  for (const ECLSimHit& t : m_eclSimHits) {
    double tof = t.getFlightTime();
    if (tof >= 8000 || tof < 0) continue;
    int TimeIndex = tof * (1. / 100);
    int cellId = t.getCellId() - 1;
    int key = cellId * 80 + TimeIndex;
    double edep = t.getEnergyDep();
    G4ThreeVector p = t.getPosition();

    const TVector3& r = eclp->GetCrystalPos(cellId);        // center of crystal position
    const TVector3& v = eclp->GetCrystalVec(cellId);        // vector of crystal axis
    double z = 15. - ((p.x() - r.X()) * v.X() + (p.y() - r.Y()) * v.Y() + (p.z() - r.Z()) *
                      v.Z()); // position along the vector of crystal axis
    double tsen = 6.05 + z * (0.0749 - z * 0.00112) + tof; // flight time to diode sensor

    hit_t& h = a[key];

    int trkid = t.getTrackId();
    tr.push_back({trkid, key});

    double old_edep = h.e, old_tsen = h.t;
    double new_edep = old_edep + edep;

    h.e = new_edep;
    h.t = (old_edep * old_tsen + edep * tsen) / new_edep;
  }

  int hitNum = m_eclHits.getEntries();
  assert(hitNum == 0);
  for (const pair<int, hit_t>&  t : a) {
    int key = t.first, cellId = key / 80;
    for (const thit_t& s : tr)
      if (s.hid == key) m_eclHitRel.add(s.tid, hitNum);
    const hit_t& h = t.second;
    m_eclHits.appendNew(cellId + 1, h.e, h.t); hitNum++;
  }
}

int SensitiveDetector::saveSimHit(G4int cellId, G4int trackID, G4int pid, G4double tof, G4double edep,
                                  const G4ThreeVector& mom, const G4ThreeVector& pos)
{
  int simhitNumber = m_eclSimHits.getEntries();
  m_eclSimHitRel.add(trackID, simhitNumber);
  tof  *= 1 / CLHEP::ns;
  edep *= 1 / CLHEP::GeV;
  m_eclSimHits.appendNew(cellId + 1, trackID, pid, tof, edep, mom * (1 / CLHEP::GeV), pos * (1 / CLHEP::cm));
  return simhitNumber;
}
