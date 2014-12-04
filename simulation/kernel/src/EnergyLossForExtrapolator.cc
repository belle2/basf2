/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 * Derived from: G4EnergyLossForExtrapolator.cc                           *
 *               (use geant4e-specific particles; include pions & kaons)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include <simulation/kernel/EnergyLossForExtrapolator.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <G4PhysicsLogVector.hh>
#include <G4ParticleDefinition.hh>
#include <G4Material.hh>
#include <G4MaterialCutsCouple.hh>
#include <G4Electron.hh>
#include <G4Positron.hh>
#include <G4Proton.hh>
#include <G4MuonPlus.hh>
#include <G4MuonMinus.hh>
#include <G4ParticleTable.hh>
#include <G4LossTableBuilder.hh>
#include <G4MollerBhabhaModel.hh>
#include <G4BetheBlochModel.hh>
#if G4VERSION_NUMBER < 1001
#include <G4eBremsstrahlungModel.hh>
#else
#include <G4eBremsstrahlungRelModel.hh>
#endif
#include <G4MuPairProductionModel.hh>
#include <G4MuBremsstrahlungModel.hh>
#include <G4ProductionCuts.hh>
#include <G4LossTableManager.hh>
#include <G4WentzelVIModel.hh>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace Belle2::Simulation;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

EnergyLossForExtrapolator::EnergyLossForExtrapolator(G4int verb)
  : maxEnergyTransfer(DBL_MAX), verbose(verb), isInitialised(false)
{
  currentParticle = 0;
  currentMaterial = 0;

  linLossLimit = 0.01;
  emin         = 1.*CLHEP::MeV;
  emax         = 10.*CLHEP::TeV;
  nbins        = 70;

  nmat = index = 0;
  cuts = 0;

  mass = charge2 = electronDensity = radLength = bg2 = beta2
                                                       = kineticEnergy = tmax = 0;
  gam = 1.0;

  dedxElectron = dedxPositron = dedxProton = rangeElectron
                                             = rangePositron = rangeProton = invRangeElectron = invRangePositron
                                                 = invRangeProton = mscElectron = dedxMuon = rangeMuon = invRangeMuon = 0;
  electron = positron = proton = muonPlus = muonMinus = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

EnergyLossForExtrapolator:: ~EnergyLossForExtrapolator()
{
  for (G4int i = 0; i < nmat; i++) {delete couples[i];}
  delete dedxElectron;
  delete dedxPositron;
  delete dedxProton;
  delete dedxMuon;
  delete dedxPion;
  delete dedxKaon;
  delete rangeElectron;
  delete rangePositron;
  delete rangeProton;
  delete rangeMuon;
  delete rangePion;
  delete rangeKaon;
  delete invRangeElectron;
  delete invRangePositron;
  delete invRangeProton;
  delete invRangeMuon;
  delete invRangePion;
  delete invRangeKaon;
  delete mscElectron;
  delete cuts;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::EnergyAfterStep(G4double kinEnergy,
                                                    G4double stepLength,
                                                    const G4Material* mat,
                                                    const G4ParticleDefinition* part)
{
  if (!isInitialised) Initialisation();
  G4double kinEnergyFinal = kinEnergy;
  if (SetupKinematics(part, mat, kinEnergy)) {
    G4double step = TrueStepLength(kinEnergy, stepLength, mat, part);
    G4double r  = ComputeRange(kinEnergy, part);
    if (r <= step) {
      kinEnergyFinal = 0.0;
    } else if (step < linLossLimit * r) {
      kinEnergyFinal -= step * ComputeDEDX(kinEnergy, part);
    } else {
      G4double r1 = r - step;
      kinEnergyFinal = ComputeEnergy(r1, part);
    }
  }
  return kinEnergyFinal;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::EnergyBeforeStep(G4double kinEnergy,
                                                     G4double stepLength,
                                                     const G4Material* mat,
                                                     const G4ParticleDefinition* part)
{
  if (!isInitialised) Initialisation();
  G4double kinEnergyFinal = kinEnergy;

  if (SetupKinematics(part, mat, kinEnergy)) {
    G4double step = TrueStepLength(kinEnergy, stepLength, mat, part);
    G4double r  = ComputeRange(kinEnergy, part);

    if (step < linLossLimit * r) {
      kinEnergyFinal += step * ComputeDEDX(kinEnergy, part);
    } else {
      G4double r1 = r + step;
      kinEnergyFinal = ComputeEnergy(r1, part);
    }
  }
  return kinEnergyFinal;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::TrueStepLength(G4double kinEnergy,
                                                   G4double stepLength,
                                                   const G4Material* mat,
                                                   const G4ParticleDefinition* part)
{
  G4double res = stepLength;
  if (!isInitialised) Initialisation();
  if (SetupKinematics(part, mat, kinEnergy)) {
    if (part == electron || part == positron) {
      G4double x = stepLength * ComputeValue(kinEnergy, mscElectron);
      if (x < 0.2) res *= (1.0 + 0.5 * x + x * x / 3.0);
      else if (x < 0.9999) res = -std::log(1.0 - x) * stepLength / x;
      else res = ComputeRange(kinEnergy, part);

    } else {
      res = ComputeTrueStep(mat, part, kinEnergy, stepLength);
    }
  }
  return res;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4bool EnergyLossForExtrapolator::SetupKinematics(const G4ParticleDefinition* part,
                                                  const G4Material* mat,
                                                  G4double kinEnergy)
{
  if (!part || !mat || kinEnergy < CLHEP::keV) return false;
  if (!isInitialised) Initialisation();
  G4bool flag = false;
  if (part != currentParticle) {
    flag = true;
    currentParticle = part;
    mass = part->GetPDGMass();
    G4double q = part->GetPDGCharge() / CLHEP::eplus;
    charge2 = q * q;
  }
  if (mat != currentMaterial) {
    G4int i = mat->GetIndex();
    if (i >= nmat) {
      B2WARNING("EnergyLossForExtrapolator: index i= " << i << " is out of table - NO extrapolation")
    } else {
      flag = true;
      currentMaterial = mat;
      electronDensity = mat->GetElectronDensity();
      radLength       = mat->GetRadlen();
      index           = i;
    }
  }
  if (flag || kinEnergy != kineticEnergy) {
    kineticEnergy = kinEnergy;
    G4double tau  = kinEnergy / mass;

    gam   = tau + 1.0;
    bg2   = tau * (tau + 2.0);
    beta2 = bg2 / (gam * gam);
    tmax  = kinEnergy;
    if (part == electron) tmax *= 0.5;
    else if (part != positron) {
      G4double r = CLHEP::electron_mass_c2 / mass;
      tmax = 2.0 * bg2 * CLHEP::electron_mass_c2 / (1.0 + 2.0 * gam * r + r * r);
    }
    if (tmax > maxEnergyTransfer) tmax = maxEnergyTransfer;
  }
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::Initialisation()
{
  isInitialised = true;
  B2DEBUG(10, "EnergyLossForExtrapolator::Initialisation")
  currentParticle = 0;
  currentMaterial = 0;
  kineticEnergy   = 0.0;

  // electron = G4Electron::Electron();
  // positron = G4Positron::Positron();
  // proton   = G4Proton::Proton();
  // muonPlus = G4MuonPlus::MuonPlus();
  // muonMinus= G4MuonMinus::MuonMinus();
  electron  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_e-");
  positron  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_e+");
  proton    = G4ParticleTable::GetParticleTable()->FindParticle("g4e_proton");
  muonPlus  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_mu+");
  muonMinus = G4ParticleTable::GetParticleTable()->FindParticle("g4e_mu-");
  pionPlus  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_pi+");
  pionMinus = G4ParticleTable::GetParticleTable()->FindParticle("g4e_pi-");
  kaonPlus  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_kaon+");
  kaonMinus = G4ParticleTable::GetParticleTable()->FindParticle("g4e_kaon-");


  currentParticleName = "";

  nmat = G4Material::GetNumberOfMaterials();
  const G4MaterialTable* mtable = G4Material::GetMaterialTable();
  cuts = new G4ProductionCuts();

  for (G4int i = 0; i < nmat; i++) {
    const G4MaterialCutsCouple* couple = new G4MaterialCutsCouple((*mtable)[i], cuts);
    couples.push_back(couple);
  }

  dedxElectron     = PrepareTable();
  dedxPositron     = PrepareTable();
  dedxMuon         = PrepareTable();
  dedxPion         = PrepareTable();
  dedxKaon         = PrepareTable();
  dedxProton       = PrepareTable();
  rangeElectron    = PrepareTable();
  rangePositron    = PrepareTable();
  rangeMuon        = PrepareTable();
  rangePion        = PrepareTable();
  rangeKaon        = PrepareTable();
  rangeProton      = PrepareTable();
  invRangeElectron = PrepareTable();
  invRangePositron = PrepareTable();
  invRangeMuon     = PrepareTable();
  invRangePion     = PrepareTable();
  invRangeKaon     = PrepareTable();
  invRangeProton   = PrepareTable();
  mscElectron      = PrepareTable();

  G4LossTableBuilder builder;

  B2DEBUG(10, "EnergyLossForExtrapolator Builds electron tables")
  ComputeElectronDEDX(electron, dedxElectron);
  builder.BuildRangeTable(dedxElectron, rangeElectron);
  builder.BuildInverseRangeTable(rangeElectron, invRangeElectron);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds positron tables")
  ComputeElectronDEDX(positron, dedxPositron);
  builder.BuildRangeTable(dedxPositron, rangePositron);
  builder.BuildInverseRangeTable(rangePositron, invRangePositron);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds muon tables")
  ComputeMuonDEDX(muonPlus, dedxMuon);
  builder.BuildRangeTable(dedxMuon, rangeMuon);
  builder.BuildInverseRangeTable(rangeMuon, invRangeMuon);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds pion tables")
  ComputeHadronDEDX(pionPlus, dedxPion);
  builder.BuildRangeTable(dedxPion, rangePion);
  builder.BuildInverseRangeTable(rangePion, invRangePion);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds kaon tables")
  ComputeHadronDEDX(kaonPlus, dedxKaon);
  builder.BuildRangeTable(dedxKaon, rangeKaon);
  builder.BuildInverseRangeTable(rangeKaon, invRangeKaon);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds proton tables")
  ComputeHadronDEDX(proton, dedxProton);
  builder.BuildRangeTable(dedxProton, rangeProton);
  builder.BuildInverseRangeTable(rangeProton, invRangeProton);

  ComputeTransportXS(electron, mscElectron);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4PhysicsTable* EnergyLossForExtrapolator::PrepareTable()
{
  G4PhysicsTable* table = new G4PhysicsTable();

  for (G4int i = 0; i < nmat; i++) {

    G4PhysicsVector* v = new G4PhysicsLogVector(emin, emax, nbins);
    v->SetSpline(G4LossTableManager::Instance()->SplineFlag());
    table->push_back(v);
  }
  return table;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

const G4ParticleDefinition* EnergyLossForExtrapolator::FindParticle(const G4String& name)
{
  const G4ParticleDefinition* p = 0;
  if (name != currentParticleName) {
    p = G4ParticleTable::GetParticleTable()->FindParticle(name);
    if (!p) {
      B2WARNING("FindParticle fails to find " << name)
    }
  } else {
    p = currentParticle;
  }
  return p;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::ComputeDEDX(G4double kinEnergy,
                                                const G4ParticleDefinition* part)
{
  G4double x = 0.0;
  if (part == electron)      x = ComputeValue(kinEnergy, dedxElectron);
  else if (part == positron) x = ComputeValue(kinEnergy, dedxPositron);
  else if (part == muonPlus || part == muonMinus)
    x = ComputeValue(kinEnergy, dedxMuon);
  else if (part == pionPlus || part == pionMinus)
    x = ComputeValue(kinEnergy, dedxPion);
  else if (part == kaonPlus || part == kaonMinus)
    x = ComputeValue(kinEnergy, dedxKaon);
  else {
    G4double e = kinEnergy * CLHEP::proton_mass_c2 / mass;
    x = ComputeValue(e, dedxProton) * charge2;
  }
  return x;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::ComputeRange(G4double kinEnergy,
                                                 const G4ParticleDefinition* part)
{
  G4double x = 0.0;
  if (part == electron)      x = ComputeValue(kinEnergy, rangeElectron);
  else if (part == positron) x = ComputeValue(kinEnergy, rangePositron);
  else if (part == muonPlus || part == muonMinus)
    x = ComputeValue(kinEnergy, rangeMuon);
  else if (part == pionPlus || part == pionMinus)
    x = ComputeValue(kinEnergy, rangePion);
  else if (part == kaonPlus || part == kaonMinus)
    x = ComputeValue(kinEnergy, rangeKaon);
  else {
    G4double massratio = CLHEP::proton_mass_c2 / mass;
    G4double e = kinEnergy * massratio;
    x = ComputeValue(e, rangeProton) / (charge2 * massratio);
  }
  return x;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::ComputeEnergy(G4double range,
                                                  const G4ParticleDefinition* part)
{
  G4double x = 0.0;
  if (part == electron)      x = ComputeValue(range, invRangeElectron);
  else if (part == positron) x = ComputeValue(range, invRangePositron);
  else if (part == muonPlus || part == muonMinus)
    x = ComputeValue(range, invRangeMuon);
  else if (part == pionPlus || part == pionMinus)
    x = ComputeValue(range, invRangePion);
  else if (part == kaonPlus || part == kaonMinus)
    x = ComputeValue(range, invRangeKaon);
  else {
    G4double massratio = CLHEP::proton_mass_c2 / mass;
    G4double r = range * massratio * charge2;
    x = ComputeValue(r, invRangeProton) / massratio;
  }
  return x;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeElectronDEDX(const G4ParticleDefinition* part,
                                                    G4PhysicsTable* table)
{
  G4DataVector v;
  G4MollerBhabhaModel* ioni = new G4MollerBhabhaModel();
#if G4VERSION_NUMBER < 1001
  G4eBremsstrahlungModel* brem = new G4eBremsstrahlungModel();
#else
  G4eBremsstrahlungRelModel* brem = new G4eBremsstrahlungRelModel();
#endif
  ioni->Initialise(part, v);
  brem->Initialise(part, v);

  mass    = CLHEP::electron_mass_c2;
  charge2 = 1.0;
  currentParticle = part;

  B2INFO("EnergyLossForExtrapolator::ComputeElectronDEDX for " << part->GetParticleName())

  for (G4int i = 0; i < nmat; i++) {

    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeElectronDEDX(): i= " << i << "  mat= " <<
            (*G4Material::GetMaterialTable())[i]->GetName())
    const G4MaterialCutsCouple* couple = couples[i];
    G4PhysicsVector* aVector = (*table)[i];

    for (G4int j = 0; j <= nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double dedx = ioni->ComputeDEDX(couple, part, e, e) + brem->ComputeDEDX(couple, part, e, e);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeElectronDEDX(): j= " << j
              << "  e(MeV)= " << e / CLHEP::MeV
              << " dedx(Mev/cm)= " << dedx * CLHEP::cm / CLHEP::MeV
              << " dedx(Mev.cm2/g)= "
              << dedx / ((CLHEP::MeV * (*G4Material::GetMaterialTable())[i]->GetDensity()) / (CLHEP::g / CLHEP::cm2)))
      aVector->PutValue(j, dedx);
    }
  }
  delete ioni;
  delete brem;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeMuonDEDX(const G4ParticleDefinition* part,
                                                G4PhysicsTable* table)
{
  G4DataVector v;
  G4BetheBlochModel* ioni = new G4BetheBlochModel();
  G4MuPairProductionModel* pair = new G4MuPairProductionModel();
  G4MuBremsstrahlungModel* brem = new G4MuBremsstrahlungModel();
  ioni->Initialise(part, v);
  pair->Initialise(part, v);
  brem->Initialise(part, v);

  mass    = part->GetPDGMass();
  charge2 = 1.0;
  currentParticle = part;

  B2INFO("EnergyLossForExtrapolator::ComputeMuonDEDX for " << part->GetParticleName())

  for (G4int i = 0; i < nmat; i++) {

    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeMuonDEDX(): i= " << i << "  mat= " <<
            (*G4Material::GetMaterialTable())[i]->GetName())
    const G4MaterialCutsCouple* couple = couples[i];
    G4PhysicsVector* aVector = (*table)[i];
    for (G4int j = 0; j <= nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double dedx = ioni->ComputeDEDX(couple, part, e, e) +
                      pair->ComputeDEDX(couple, part, e, e) +
                      brem->ComputeDEDX(couple, part, e, e);
      aVector->PutValue(j, dedx);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeMuonDEDX(): j= " << j
              << "  e(MeV)= " << e / CLHEP::MeV
              << " dedx(Mev/cm)= " << dedx * CLHEP::cm / CLHEP::MeV
              << " dedx(Mev/(g/cm2)= "
              << dedx / ((CLHEP::MeV * (*G4Material::GetMaterialTable())[i]->GetDensity()) / (CLHEP::g / CLHEP::cm2)))
    }
  }
  delete ioni;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeHadronDEDX(const G4ParticleDefinition* part,
                                                  G4PhysicsTable* table)
{
  G4DataVector v;
  G4BetheBlochModel* ioni = new G4BetheBlochModel();
  ioni->Initialise(part, v);

  mass    = part->GetPDGMass();
  charge2 = 1.0;
  currentParticle = part;

  B2INFO("EnergyLossForExtrapolator::ComputeHadronDEDX for " << part->GetParticleName())

  for (G4int i = 0; i < nmat; i++) {

    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeHadronDEDX(): i= " << i << "  mat= " <<
            (*G4Material::GetMaterialTable())[i]->GetName())
    const G4MaterialCutsCouple* couple = couples[i];
    G4PhysicsVector* aVector = (*table)[i];
    for (G4int j = 0; j <= nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double dedx = ioni->ComputeDEDX(couple, part, e, e);
      aVector->PutValue(j, dedx);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeHadronDEDX(): j= " << j
              << "  e(MeV)= " << e / CLHEP::MeV
              << " dedx(Mev/cm)= " << dedx * CLHEP::cm / CLHEP::MeV
              << " dedx(Mev.cm2/g)= "
              << dedx / (((*G4Material::GetMaterialTable())[i]->GetDensity()) / (CLHEP::g / CLHEP::cm2)))
    }
  }
  delete ioni;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeTransportXS(const G4ParticleDefinition* part,
                                                   G4PhysicsTable* table)
{
  G4DataVector v;
  G4WentzelVIModel* msc = new G4WentzelVIModel();
  msc->SetPolarAngleLimit(CLHEP::pi);
  msc->Initialise(part, v);

  mass    = part->GetPDGMass();
  charge2 = 1.0;
  currentParticle = part;

  const G4MaterialTable* mtable = G4Material::GetMaterialTable();
  B2INFO("EnergyLossForExtrapolator::ComputeTransportXS for " << part->GetParticleName())

  for (G4int i = 0; i < nmat; i++) {

    const G4Material* mat = (*mtable)[i];
    msc->SetCurrentCouple(couples[i]);
    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeTransportXS(): i= " << i << "  mat= " << mat->GetName())
    G4PhysicsVector* aVector = (*table)[i];
    for (G4int j = 0; j <= nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double xs = msc->CrossSectionPerVolume(mat, part, e);
      aVector->PutValue(j, xs);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeTransportXS(): j= " << j << "  e(MeV)= " << e / CLHEP::MeV
              << " xs(1/mm)= " << xs * CLHEP::mm)
    }
  }
  delete msc;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

