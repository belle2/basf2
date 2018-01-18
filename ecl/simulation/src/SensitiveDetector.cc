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
#include <framework/gearbox/Unit.h>
#include <simulation/background/BkgNeutronWeight.h>

#include "G4EmCalculator.hh"// Used to get dE/dx for pulse shape construction
#include <framework/utilities/FileSystem.h>

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
  m_hadronenergyDeposit = 0;

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
  if (not m_HadronEmissionFunction) {
    m_HadronEmissionFile = FileSystem::findFile("/data/ecl/HadronScintEmissionFunction.root");
    TFile* InFile = new TFile(m_HadronEmissionFile.c_str(), "READ");
    if (!InFile || InFile->IsZombie())
      B2FATAL("Could not open file " << "HadronScintEmissionFunction.root");
    m_HadronEmissionFunction = (TGraph*) InFile->Get("HadronEmissionFunction");
    InFile->Close();
    delete InFile;
  }
}
//Returns percent of scintillation emission for hadron component for given ionization dEdx value
//See slides: https://kds.kek.jp/indico/event/24563/session/17/contribution/256/material/slides/0.pdf
//for additional details
double SensitiveDetector::GetHadronIntensityFromDEDX(double x)
{
  if (x < 2) return 0;
  if (x > 232) return m_HadronEmissionFunction->Eval(232);
  return m_HadronEmissionFunction->Eval(x);
}
//
//Return total scintillation efficiency, normalized to 1 for photons, for CsI(Tl) given ionization dEdx value.
//See slides: https://kds.kek.jp/indico/event/24563/session/17/contribution/256/material/slides/0.pdf
//for additional details. Parameters in function below are for blue dL/dE curve on bottom right of slide 11.
double GetCsITlScintillationEfficiency(double x)
{
  const double p0 = 1.52;
  const double p1 = 0.00344839;
  const double p2 = 2.0;
  double val = 1;
  if (x > 3.9406) {
    val = (x * p0) / (x + (p1 * x * x) + p2);
  }
  return val;
}
//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
{
  //
  G4EmCalculator emCal;
  double preKineticEnergy = aStep->GetPreStepPoint()->GetKineticEnergy();
  double postKineticEnergy = aStep->GetPostStepPoint()->GetKineticEnergy();
  double avgKineticEnergy = 0.5 * (preKineticEnergy + postKineticEnergy);
  const G4ParticleDefinition* StepParticleDefinition = aStep->GetTrack()->GetParticleDefinition();
  G4Material* StepMaterial = aStep->GetTrack()->GetMaterial();
  const double CsIDensity = 4.51; //gcm^-3
  double ELE_DEDX = emCal.ComputeDEDX(avgKineticEnergy, StepParticleDefinition, "eIoni"  ,
                                      StepMaterial) / CLHEP::MeV * CLHEP::cm / (CsIDensity);
  double HAD_DEDX = emCal.ComputeDEDX(avgKineticEnergy, StepParticleDefinition, "hIoni"  ,
                                      StepMaterial) / CLHEP::MeV * CLHEP::cm / (CsIDensity);
  double ION_DEDX = emCal.ComputeDEDX(avgKineticEnergy, StepParticleDefinition, "ionIoni",
                                      StepMaterial) / CLHEP::MeV * CLHEP::cm / (CsIDensity);
  G4double DEDX_val = ELE_DEDX + HAD_DEDX + ION_DEDX; //Ionization dE/dx for any particle type
  //
  //  return true;
  G4double edep = aStep->GetTotalEnergyDeposit();
  double LightOutputCorrection = GetCsITlScintillationEfficiency(DEDX_val);
  edep *= LightOutputCorrection;
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
    m_hadronenergyDeposit = 0;
  }
  //Update energy deposit
  G4double hedep = 0.5 * edep; // half of energy deposition -- for averaging purpose
  m_energyDeposit += edep;
  m_WeightedTime  += (s0.GetGlobalTime() + s1.GetGlobalTime()) * hedep;
  m_WeightedPos   += (s0.GetPosition()   + s1.GetPosition()) * hedep;
  //
  //Calculate hadronic scintillation component intensity from dEdx
  m_hadronenergyDeposit += (edep / CLHEP::GeV) * GetHadronIntensityFromDEDX(DEDX_val);
  //
  //Save Hit if track leaves volume or is killed
  if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
    if (m_energyDeposit > 0.0) {
      ECLGeometryPar* eclp = ECLGeometryPar::Instance();
      G4int cellID = eclp->TouchableToCellID(s0.GetTouchable());
      G4double dTotalEnergy = 1 / m_energyDeposit;
      m_WeightedTime *= dTotalEnergy;
      m_WeightedPos  *= dTotalEnergy;
      int pdgCode = track.GetDefinition()->GetPDGEncoding();
      saveSimHit(cellID, m_trackID, pdgCode, m_WeightedTime, m_energyDeposit, m_momentum, m_WeightedPos, m_hadronenergyDeposit);
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
    if (tof >= 8000.0 || tof < -8000.0) continue;
    int TimeIndex = (tof + 8000.0) * (1. / 100);
    int cellId = t.getCellId() - 1;
    int key = cellId * 160 + TimeIndex;
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
  //  assert(hitNum == 0);
  for (const pair<int, hit_t>&  t : a) {
    int key = t.first, cellId = key / 160;
    for (const thit_t& s : tr)
      if (s.hid == key) m_eclHitRel.add(s.tid, hitNum);
    const hit_t& h = t.second;
    m_eclHits.appendNew(cellId + 1, h.e, h.t); hitNum++;
  }
}

int SensitiveDetector::saveSimHit(G4int cellId, G4int trackID, G4int pid, G4double tof, G4double edep,
                                  const G4ThreeVector& mom, const G4ThreeVector& pos, double Hadronedep)
{
  int simhitNumber = m_eclSimHits.getEntries();
  m_eclSimHitRel.add(trackID, simhitNumber);
  tof  *= 1 / CLHEP::ns;
  edep *= 1 / CLHEP::GeV;
  m_eclSimHits.appendNew(cellId + 1, trackID, pid, tof, edep, mom * (1 / CLHEP::GeV), pos * (1 / CLHEP::cm), Hadronedep);
  return simhitNumber;
}

SensitiveDiode::SensitiveDiode(const G4String& name):
  Simulation::SensitiveDetectorBase(name, Const::ECL)
{
  m_eclHits.registerInDataStore("ECLDiodeHits");
  m_eclp = ECLGeometryPar::Instance();
  m_trackID = -1;
  m_tsum = 0;
  m_esum = 0;
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
    v.clear();
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

BkgSensitiveDiode::BkgSensitiveDiode(const G4String& name):
  Simulation::SensitiveDetectorBase(name, Const::invalidDetector),
  m_eclBeamBkgHitRel(m_mcParticles, m_eclBeamBkgHits)
{
  registerMCParticleRelation(m_eclBeamBkgHitRel);
  m_eclBeamBkgHits.registerInDataStore();
  m_mcParticles.registerRelationTo(m_eclBeamBkgHits);

  m_eclp = ECLGeometryPar::Instance();
  m_energyDeposit = 0;
  m_startEnergy = 0;
  m_startTime = 0;
  m_trackLength = 0;
  m_trackID = -1;
}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
bool BkgSensitiveDiode::step(G4Step* aStep, G4TouchableHistory*)
{
  const G4StepPoint& s0 = *aStep->GetPreStepPoint();
  const G4Track&  track = *aStep->GetTrack();

  if (m_trackID != track.GetTrackID()) { //TrackID changed, store track informations
    m_trackID = track.GetTrackID();
    //Get world position
    const G4ThreeVector& worldPosition = s0.GetPosition();
    const double mm2cm = Unit::mm / Unit::cm;
    m_startPos.SetXYZ(worldPosition.x() * mm2cm , worldPosition.y() * mm2cm, worldPosition.z() * mm2cm);
    //Get momentum
    const G4ThreeVector& momentum = s0.GetMomentum() ;
    m_startMom.SetXYZ(momentum.x() * Unit::MeV, momentum.y() * Unit::MeV, momentum.z() * Unit::MeV);
    //Get time
    m_startTime = s0.GetGlobalTime();
    //Get energy
    m_startEnergy = s0.GetKineticEnergy() * Unit::MeV;
    //Reset energy deposit;
    m_energyDeposit = 0;
    //Reset track lenght;
    m_trackLength = 0;
  }
  //Update energy deposit
  m_energyDeposit += aStep->GetTotalEnergyDeposit() * Unit::MeV;
  m_trackLength += aStep->GetStepLength() * Unit::mm;
  //Save Hit if track leaves volume or is killed
  if (track.GetNextVolume() != track.GetVolume() ||
      track.GetTrackStatus() >= fStopAndKill) {
    int pdgCode = track.GetDefinition()->GetPDGEncoding();
    double endEnergy = track.GetKineticEnergy() * Unit::MeV;
    double neutWeight = 0;
    if (pdgCode == 2112) {
      BkgNeutronWeight& wt = BkgNeutronWeight::getInstance();
      neutWeight = wt.getWeight(m_startEnergy / Unit::MeV);
    }

    int bkgHitNumber = m_eclBeamBkgHits.getEntries();
    m_eclBeamBkgHitRel.add(m_trackID, bkgHitNumber);

    int cellID = m_eclp->TouchableDiodeToCellID(s0.GetTouchable());
    m_eclBeamBkgHits.appendNew(6, cellID, pdgCode, m_trackID, m_startPos, m_startMom, m_startTime, endEnergy, m_startEnergy,
                               m_energyDeposit, m_trackLength, neutWeight);

    //Reset TrackID
    m_trackID = 0;
  }
  return true;
}
