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
    double tsen = tof + eclp->time2sensor(cellId, t.getPosition()); // flight time to diode sensor

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

SensitiveDiode::SensitiveDiode(const G4String& name):
  Simulation::SensitiveDetectorBase(name, Const::ECL)
{
  m_eclHits.registerInDataStore("ECLDiodeHits");
  m_eclp = ECLGeometryPar::Instance();
}

SensitiveDiode::~SensitiveDiode()
{
}

void SensitiveDiode::Initialize(G4HCofThisEvent*)
{
  m_hits.clear();
  m_cells.clear();
}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
bool SensitiveDiode::step(G4Step* aStep, G4TouchableHistory*)
{
  const G4StepPoint& s0 = *aStep->GetPreStepPoint();
  const G4StepPoint& s1 = *aStep->GetPostStepPoint();
  const G4Track&  track = *aStep->GetTrack();

  if (m_trackID != track.GetTrackID()) { //TrackID changed, store track informations
    m_trackID = track.GetTrackID();
    //Reset track parameters
    m_esum = 0;
    m_tsum = 0;
  }
  //Update energy deposit
  G4double edep = aStep->GetTotalEnergyDeposit();
  m_esum += edep;
  m_tsum += (s0.GetGlobalTime() + s1.GetGlobalTime()) * edep;
  //Save Hit if track leaves volume or is killed
  if (track.GetNextVolume() != track.GetVolume() ||
      track.GetTrackStatus() >= fStopAndKill) {
    if (m_esum > 0.0) {
      int cellID = m_eclp->TouchableDiodeToCellID(s0.GetTouchable());
      m_tsum /= 2 * m_esum; // average
      m_hits.push_back({cellID, m_esum, m_tsum});
      auto it = find(m_cells.begin(), m_cells.end(), cellID);
      if (it == m_cells.end())m_cells.push_back(cellID);
#if 0
      const G4ThreeVector& p = s0.GetPosition();
      G4ThreeVector cp = 10 * m_eclp->getCrystalPos(cellID);
      G4ThreeVector cv = m_eclp->getCrystalVec(cellID);
      double dz = (p - cp) * cv;
      double rho = ((p - cp) - dz * cv).perp();
      cout << "diff " << cellID << " " << dz << " " << rho << endl;
      if (abs(dz - 150) > 1) {
        const G4VTouchable* touch = s0.GetTouchable();
        const G4NavigationHistory* h = touch->GetHistory();
        int hd = h->GetDepth();
        int i1 = h->GetReplicaNo(hd - 1); // index of each volume is set at physical volume creation
        int i2 = h->GetReplicaNo(hd - 2); // go up in volume hierarchy
        const G4String& vname = touch->GetVolume()->GetName();
        cout << vname << " " << hd << " " << i1 << " " << i2 << endl;
        for (int i = 0; i <= hd; i++) {
          G4VPhysicalVolume* v = h->GetVolume(i);
          cout << v->GetName() << endl;
        }
      }
#endif
    }
    //Reset TrackID
    m_trackID = 0;
  }
  return true;
}

void SensitiveDiode::EndOfEvent(G4HCofThisEvent*)
{
  struct dep_t {double e, t;};
  vector<dep_t> v;
  for (int cellId : m_cells) {
    for (const hit_t& h : m_hits) {
      if (cellId == h.cellId) v.push_back({h.e, h.t});
    }

    double esum = 0, tsum = 0;
    for (const dep_t& t : v) {
      esum += t.e;
      tsum += t.t * t.e;
    }
    tsum /= esum;

    double trms = 0;
    for (const dep_t& t : v) trms += pow(t.t - tsum, 2) * t.e ;
    trms /= esum;

    tsum *= 1 / CLHEP::ns;
    esum *= 1 / CLHEP::GeV;
    m_eclHits.appendNew(cellId + 1, esum, tsum);


    // cout<<"DD: "<<cellId<<" "<<esum<<" "<<tsum<<" +- "<<sqrt(tsum2)<<endl;

    // sort(v.begin(),v.end(),[](const dep_t &a, const dep_t &b){return a.t<b.t;});
    // cout<<"DD: "<<cellId<<" ";
    // for(const dep_t &t: v)cout<<t.e<<" MeV "<<t.t<<" nsec, ";
    // cout<<"\n";

  }
}
