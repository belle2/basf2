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
#include <G4ParticleTable.hh>
#include <G4LossTableBuilder.hh>
#include <G4MollerBhabhaModel.hh>
#include <G4BetheBlochModel.hh>
#include <G4Version.hh>
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

EnergyLossForExtrapolator::EnergyLossForExtrapolator(void)
  : m_UserMaxEnergyTransfer(DBL_MAX), m_Initialised(false)
{
  m_Particle = 0;
  m_Material = 0;

  m_LinLossLimit = 0.01;
  m_UserTmin = 1.*CLHEP::MeV;
  m_UserTmax = 10.*CLHEP::TeV;
  m_Nbins = 70;

  m_NMaterials = m_MaterialIndex = 0;
  m_ProductionCuts = 0;

  m_Mass = m_ChargeSq = m_ElectronDensity = m_RadLength = m_BetaGammaSq = m_BetaSq = m_KineticEnergy = m_Tmax = 0;
  m_Gamma = 1.0;

  m_Electron = m_Positron = m_MuonPlus = m_MuonMinus =
                                           m_PionPlus = m_PionMinus = m_KaonPlus = m_KaonMinus =
                                                          m_Proton = m_AntiProton = m_Deuteron = m_AntiDeuteron = 0;
  m_DedxElectron = m_RangeElectron = m_InvRangeElectron = m_MscatElectron = 0;
  m_DedxPositron = m_RangePositron = m_InvRangePositron = 0;
  m_DedxMuon = m_RangeMuon = m_InvRangeMuon = 0;
  m_DedxPion = m_RangePion = m_InvRangePion = 0;
  m_DedxKaon = m_RangeKaon = m_InvRangeKaon = 0;
  m_DedxProton = m_RangeProton = m_InvRangeProton = 0;
  m_DedxDeuteron = m_RangeDeuteron = m_InvRangeDeuteron = 0;

  Initialisation();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

EnergyLossForExtrapolator:: ~EnergyLossForExtrapolator()
{
  m_DedxElectron->clearAndDestroy(); delete m_DedxElectron;
  m_DedxPositron->clearAndDestroy(); delete m_DedxPositron;
  m_DedxMuon->clearAndDestroy(); delete m_DedxMuon;
  m_DedxPion->clearAndDestroy(); delete m_DedxPion;
  m_DedxKaon->clearAndDestroy(); delete m_DedxKaon;
  m_DedxProton->clearAndDestroy(); delete m_DedxProton;
  m_DedxDeuteron->clearAndDestroy(); delete m_DedxDeuteron;
  m_RangeElectron->clearAndDestroy(); delete m_RangeElectron;
  m_RangePositron->clearAndDestroy(); delete m_RangePositron;
  m_RangeMuon->clearAndDestroy(); delete m_RangeMuon;
  m_RangePion->clearAndDestroy(); delete m_RangePion;
  m_RangeKaon->clearAndDestroy(); delete m_RangeKaon;
  m_RangeProton->clearAndDestroy(); delete m_RangeProton;
  m_RangeDeuteron->clearAndDestroy(); delete m_RangeDeuteron;
  m_InvRangeElectron->clearAndDestroy(); delete m_InvRangeElectron;
  m_InvRangePositron->clearAndDestroy(); delete m_InvRangePositron;
  m_InvRangeMuon->clearAndDestroy(); delete m_InvRangeMuon;
  m_InvRangePion->clearAndDestroy(); delete m_InvRangePion;
  m_InvRangeKaon->clearAndDestroy(); delete m_InvRangeKaon;
  m_InvRangeProton->clearAndDestroy(); delete m_InvRangeProton;
  m_InvRangeDeuteron->clearAndDestroy(); delete m_InvRangeDeuteron;
  m_MscatElectron->clearAndDestroy(); delete m_MscatElectron;
  delete m_ProductionCuts;
  for (const G4MaterialCutsCouple* couple : m_Couples) delete couple;
  m_Couples.clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::EnergyAfterStep(G4double kinEnergy,
                                                    G4double stepLength,
                                                    const G4Material* mat,
                                                    const G4ParticleDefinition* part)
{
  //- if (!isInitialised) Initialisation();
  G4double kinEnergyFinal = kinEnergy;
  if (SetupKinematics(part, mat, kinEnergy)) {
    G4double step = TrueStepLength(kinEnergy, stepLength, mat, part);
    G4double r  = ComputeRange(kinEnergy, part);
    if (r <= step) {
      kinEnergyFinal = 0.0;
    } else if (step < m_LinLossLimit * r) {
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
  //- if (!isInitialised) Initialisation();
  G4double kinEnergyFinal = kinEnergy;

  if (SetupKinematics(part, mat, kinEnergy)) {
    G4double step = TrueStepLength(kinEnergy, stepLength, mat, part);
    G4double r  = ComputeRange(kinEnergy, part);

    if (step < m_LinLossLimit * r) {
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
  //- if (!isInitialised) Initialisation();
  if (SetupKinematics(part, mat, kinEnergy)) {
    if (part == m_Electron || part == m_Positron) {
      G4double x = stepLength * ComputeValue(kinEnergy, m_MscatElectron);
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
  //- if (!isInitialised) Initialisation();
  G4bool flag = false;
  if (part != m_Particle) {
    flag = true;
    m_Particle = part;
    m_Mass = part->GetPDGMass();
    G4double q = part->GetPDGCharge() / CLHEP::eplus;
    m_ChargeSq = q * q;
  }
  if (mat != m_Material) {
    G4int i = mat->GetIndex();
    if (i >= m_NMaterials) {
      B2WARNING("EnergyLossForExtrapolator: index i= " << i << " is out of table - NO extrapolation");
    } else {
      flag = true;
      m_Material = mat;
      m_ElectronDensity = mat->GetElectronDensity();
      m_RadLength = mat->GetRadlen();
      m_MaterialIndex = i;
    }
  }
  if (flag || kinEnergy != m_KineticEnergy) {
    m_KineticEnergy = kinEnergy;
    G4double tau  = kinEnergy / m_Mass;

    m_Gamma = tau + 1.0;
    m_BetaGammaSq = tau * (tau + 2.0);
    m_BetaSq = m_BetaGammaSq / (m_Gamma * m_Gamma);
    m_Tmax  = kinEnergy;
    if (part == m_Electron) m_Tmax *= 0.5;
    else if (part != m_Positron) {
      G4double r = CLHEP::electron_mass_c2 / m_Mass;
      m_Tmax = 2.0 * m_BetaGammaSq * CLHEP::electron_mass_c2 / (1.0 + 2.0 * m_Gamma * r + r * r);
    }
    if (m_Tmax > m_UserMaxEnergyTransfer) m_Tmax = m_UserMaxEnergyTransfer;
  }
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::Initialisation()
{
  m_Initialised = true;
  B2DEBUG(10, "EnergyLossForExtrapolator::Initialisation");
  m_Particle = 0;
  m_Material = 0;
  m_KineticEnergy = 0.0;

  m_Electron  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_e-");
  m_Positron  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_e+");
  m_MuonPlus  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_mu+");
  m_MuonMinus = G4ParticleTable::GetParticleTable()->FindParticle("g4e_mu-");
  m_PionPlus  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_pi+");
  m_PionMinus = G4ParticleTable::GetParticleTable()->FindParticle("g4e_pi-");
  m_KaonPlus  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_kaon+");
  m_KaonMinus = G4ParticleTable::GetParticleTable()->FindParticle("g4e_kaon-");
  m_Proton  = G4ParticleTable::GetParticleTable()->FindParticle("g4e_proton");
  m_AntiProton = G4ParticleTable::GetParticleTable()->FindParticle("g4e_anti_proton");
  m_Deuteron = G4ParticleTable::GetParticleTable()->FindParticle("g4e_deuteron");
  m_AntiDeuteron = G4ParticleTable::GetParticleTable()->FindParticle("g4e_anti_deuteron");

  m_NMaterials = G4Material::GetNumberOfMaterials();
  const G4MaterialTable* mtable = G4Material::GetMaterialTable();
  m_ProductionCuts = new G4ProductionCuts();

  m_Couples.resize(m_NMaterials, 0);
  m_Cuts.resize(m_NMaterials, DBL_MAX);

  for (G4int i = 0; i < m_NMaterials; i++) {
    m_Couples[i] = new G4MaterialCutsCouple((*mtable)[i], m_ProductionCuts);
  }

  m_DedxElectron     = PrepareTable();
  m_DedxPositron     = PrepareTable();
  m_DedxMuon         = PrepareTable();
  m_DedxPion         = PrepareTable();
  m_DedxKaon         = PrepareTable();
  m_DedxProton       = PrepareTable();
  m_DedxDeuteron     = PrepareTable();
  m_RangeElectron    = PrepareTable();
  m_RangePositron    = PrepareTable();
  m_RangeMuon        = PrepareTable();
  m_RangePion        = PrepareTable();
  m_RangeKaon        = PrepareTable();
  m_RangeProton      = PrepareTable();
  m_RangeDeuteron    = PrepareTable();
  m_InvRangeElectron = PrepareTable();
  m_InvRangePositron = PrepareTable();
  m_InvRangeMuon     = PrepareTable();
  m_InvRangePion     = PrepareTable();
  m_InvRangeKaon     = PrepareTable();
  m_InvRangeProton   = PrepareTable();
  m_InvRangeDeuteron = PrepareTable();
  m_MscatElectron    = PrepareTable();

  G4LossTableBuilder builder;

  B2DEBUG(10, "EnergyLossForExtrapolator Builds electron tables");
  ComputeElectronDEDX(m_Electron, m_DedxElectron);
  builder.BuildRangeTable(m_DedxElectron, m_RangeElectron);
  builder.BuildInverseRangeTable(m_RangeElectron, m_InvRangeElectron);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds positron tables");
  ComputeElectronDEDX(m_Positron, m_DedxPositron);
  builder.BuildRangeTable(m_DedxPositron, m_RangePositron);
  builder.BuildInverseRangeTable(m_RangePositron, m_InvRangePositron);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds muon tables");
  ComputeMuonDEDX(m_MuonPlus, m_DedxMuon);
  builder.BuildRangeTable(m_DedxMuon, m_RangeMuon);
  builder.BuildInverseRangeTable(m_RangeMuon, m_InvRangeMuon);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds pion tables");
  ComputeHadronDEDX(m_PionPlus, m_DedxPion);
  builder.BuildRangeTable(m_DedxPion, m_RangePion);
  builder.BuildInverseRangeTable(m_RangePion, m_InvRangePion);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds kaon tables");
  ComputeHadronDEDX(m_KaonPlus, m_DedxKaon);
  builder.BuildRangeTable(m_DedxKaon, m_RangeKaon);
  builder.BuildInverseRangeTable(m_RangeKaon, m_InvRangeKaon);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds proton tables");
  ComputeHadronDEDX(m_Proton, m_DedxProton);
  builder.BuildRangeTable(m_DedxProton, m_RangeProton);
  builder.BuildInverseRangeTable(m_RangeProton, m_InvRangeProton);

  B2DEBUG(10, "EnergyLossForExtrapolator Builds deuteron tables");
  ComputeHadronDEDX(m_Deuteron, m_DedxDeuteron);
  builder.BuildRangeTable(m_DedxDeuteron, m_RangeDeuteron);
  builder.BuildInverseRangeTable(m_RangeDeuteron, m_InvRangeDeuteron);

  ComputeTransportXS(m_Electron, m_MscatElectron);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4PhysicsTable* EnergyLossForExtrapolator::PrepareTable()
{
  G4PhysicsTable* table = new G4PhysicsTable();

  for (G4int i = 0; i < m_NMaterials; i++) {

    G4PhysicsVector* v = new G4PhysicsLogVector(m_UserTmin, m_UserTmax, m_Nbins);
    v->SetSpline(true);
    table->push_back(v);
  }
  return table;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::ComputeDEDX(G4double kinEnergy,
                                                const G4ParticleDefinition* part)
{
  G4double x = 0.0;
  if (part == m_Electron) {
    x = ComputeValue(kinEnergy, m_DedxElectron);
  } else if (part == m_Positron) {
    x = ComputeValue(kinEnergy, m_DedxPositron);
  } else if (part == m_MuonPlus || part == m_MuonMinus) {
    x = ComputeValue(kinEnergy, m_DedxMuon);
  } else if (part == m_PionPlus || part == m_PionMinus) {
    x = ComputeValue(kinEnergy, m_DedxPion);
  } else if (part == m_KaonPlus || part == m_KaonMinus) {
    x = ComputeValue(kinEnergy, m_DedxKaon);
  } else if (part == m_Proton || part == m_AntiProton) {
    x = ComputeValue(kinEnergy, m_DedxProton);
  } else if (part == m_Deuteron || part == m_AntiDeuteron) {
    x = ComputeValue(kinEnergy, m_DedxDeuteron);
  } else {
    B2FATAL("EnergyLossForExtrapolator::ComputeDEDX has no table for particle " << part->GetParticleName());
  }
  return x;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::ComputeRange(G4double kinEnergy,
                                                 const G4ParticleDefinition* part)
{
  G4double x = 0.0;
  if (part == m_Electron) {
    x = ComputeValue(kinEnergy, m_RangeElectron);
  } else if (part == m_Positron) {
    x = ComputeValue(kinEnergy, m_RangePositron);
  } else if (part == m_MuonPlus || part == m_MuonMinus) {
    x = ComputeValue(kinEnergy, m_RangeMuon);
  } else if (part == m_PionPlus || part == m_PionMinus) {
    x = ComputeValue(kinEnergy, m_RangePion);
  } else if (part == m_KaonPlus || part == m_KaonMinus) {
    x = ComputeValue(kinEnergy, m_RangeKaon);
  } else if (part == m_Proton || part == m_AntiProton) {
    x = ComputeValue(kinEnergy, m_RangeProton);
  } else if (part == m_Deuteron || part == m_AntiDeuteron) {
    x = ComputeValue(kinEnergy, m_RangeDeuteron);
  } else {
    B2FATAL("EnergyLossForExtrapolator::ComputeRange has no table for particle " << part->GetParticleName());
  }
  return x;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double EnergyLossForExtrapolator::ComputeEnergy(G4double range,
                                                  const G4ParticleDefinition* part)
{
  G4double x = 0.0;
  if (part == m_Electron) {
    x = ComputeValue(range, m_InvRangeElectron);
  } else if (part == m_Positron) {
    x = ComputeValue(range, m_InvRangePositron);
  } else if (part == m_MuonPlus || part == m_MuonMinus) {
    x = ComputeValue(range, m_InvRangeMuon);
  } else if (part == m_PionPlus || part == m_PionMinus) {
    x = ComputeValue(range, m_InvRangePion);
  } else if (part == m_KaonPlus || part == m_KaonMinus) {
    x = ComputeValue(range, m_InvRangeKaon);
  } else if (part == m_Proton || part == m_AntiProton) {
    x = ComputeValue(range, m_InvRangeProton);
  } else if (part == m_Deuteron || part == m_AntiDeuteron) {
    x = ComputeValue(range, m_InvRangeDeuteron);
  } else {
    B2FATAL("EnergyLossForExtrapolator::ComputeEnergy has no table for particle " << part->GetParticleName());
  }
  return x;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeElectronDEDX(const G4ParticleDefinition* part,
                                                    G4PhysicsTable* table)
{
  G4MollerBhabhaModel* ioni = new G4MollerBhabhaModel();
#if G4VERSION_NUMBER < 1001
  G4eBremsstrahlungModel* brem = new G4eBremsstrahlungModel();
#else
  G4eBremsstrahlungRelModel* brem = new G4eBremsstrahlungRelModel();
#endif
  G4ParticleChange* ioniPC = new G4ParticleChange();
  ioni->SetParticleChange(ioniPC);
  G4ParticleChange* bremPC = new G4ParticleChange();
  brem->SetParticleChange(bremPC);

  ioni->Initialise(part, m_Cuts);
  brem->Initialise(part, m_Cuts);

  m_Mass = CLHEP::electron_mass_c2;
  m_ChargeSq = 1.0;
  m_Particle = part;

  B2DEBUG(1, "EnergyLossForExtrapolator::ComputeElectronDEDX for " << part->GetParticleName());

  for (G4int i = 0; i < m_NMaterials; i++) {

    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeElectronDEDX(): i= " << i << "  mat= " <<
            (*G4Material::GetMaterialTable())[i]->GetName());
    const G4MaterialCutsCouple* couple = m_Couples[i];
    G4PhysicsVector* aVector = (*table)[i];

    for (G4int j = 0; j <= m_Nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double dedx = ioni->ComputeDEDX(couple, part, e, e) +
                      brem->ComputeDEDX(couple, part, e, e);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeElectronDEDX(): j= " << j
              << "  e(MeV)= " << e / CLHEP::MeV
              << " dedx(Mev/cm)= " << dedx * CLHEP::cm / CLHEP::MeV
              << " dedx(Mev.cm2/g)= "
              << dedx / ((CLHEP::MeV * (*G4Material::GetMaterialTable())[i]->GetDensity()) / (CLHEP::g / CLHEP::cm2)));
      aVector->PutValue(j, dedx);
    }
    aVector->FillSecondDerivatives();
  }
  delete ioni; delete ioniPC;
  delete brem; delete bremPC;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeMuonDEDX(const G4ParticleDefinition* part,
                                                G4PhysicsTable* table)
{
  G4BetheBlochModel* ioni = new G4BetheBlochModel();
  G4MuPairProductionModel* pair = new G4MuPairProductionModel();
  G4MuBremsstrahlungModel* brem = new G4MuBremsstrahlungModel();

  G4ParticleChange* ioniPC = new G4ParticleChange();
  ioni->SetParticleChange(ioniPC);
  G4ParticleChange* pairPC = new G4ParticleChange();
  pair->SetParticleChange(pairPC);
  G4ParticleChange* bremPC = new G4ParticleChange();
  brem->SetParticleChange(bremPC);

  ioni->Initialise(part, m_Cuts);
  pair->Initialise(part, m_Cuts);
  brem->Initialise(part, m_Cuts);

  m_Mass = part->GetPDGMass();
  m_ChargeSq = 1.0;
  m_Particle = part;

  B2DEBUG(1, "EnergyLossForExtrapolator::ComputeMuonDEDX for " << part->GetParticleName());

  for (G4int i = 0; i < m_NMaterials; i++) {

    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeMuonDEDX(): i= " << i << "  mat= " <<
            (*G4Material::GetMaterialTable())[i]->GetName());
    const G4MaterialCutsCouple* couple = m_Couples[i];
    G4PhysicsVector* aVector = (*table)[i];
    for (G4int j = 0; j <= m_Nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double dedx = ioni->ComputeDEDX(couple, part, e, e) +
                      pair->ComputeDEDX(couple, part, e, e) +
                      brem->ComputeDEDX(couple, part, e, e);
      aVector->PutValue(j, dedx);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeMuonDEDX(): j= " << j
              << "  e(MeV)= " << e / CLHEP::MeV
              << " dedx(Mev/cm)= " << dedx * CLHEP::cm / CLHEP::MeV
              << " dedx(Mev/(g/cm2)= "
              << dedx / ((CLHEP::MeV * (*G4Material::GetMaterialTable())[i]->GetDensity()) / (CLHEP::g / CLHEP::cm2)));
    }
    aVector->FillSecondDerivatives();
  }
  delete ioni; delete ioniPC;
  delete pair; delete pairPC;
  delete brem; delete bremPC;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeHadronDEDX(const G4ParticleDefinition* part,
                                                  G4PhysicsTable* table)
{
  G4BetheBlochModel* ioni = new G4BetheBlochModel();
  G4ParticleChange* ioniPC = new G4ParticleChange();
  ioni->SetParticleChange(ioniPC);
  ioni->Initialise(part, m_Cuts);

  m_Mass = part->GetPDGMass();
  double q = part->GetPDGCharge() / CLHEP::eplus;
  m_ChargeSq = q * q;
  m_Particle = part;

  B2DEBUG(1, "EnergyLossForExtrapolator::ComputeHadronDEDX for " << part->GetParticleName());

  for (G4int i = 0; i < m_NMaterials; i++) {

    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeHadronDEDX(): i= " << i << "  mat= " <<
            (*G4Material::GetMaterialTable())[i]->GetName());
    const G4MaterialCutsCouple* couple = m_Couples[i];
    G4PhysicsVector* aVector = (*table)[i];
    for (G4int j = 0; j <= m_Nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double dedx = ioni->ComputeDEDX(couple, part, e, e);
      aVector->PutValue(j, dedx);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeHadronDEDX(): j= " << j
              << "  e(MeV)= " << e / CLHEP::MeV
              << " dedx(Mev/cm)= " << dedx * CLHEP::cm / CLHEP::MeV
              << " dedx(Mev.cm2/g)= "
              << dedx / (((*G4Material::GetMaterialTable())[i]->GetDensity()) / (CLHEP::g / CLHEP::cm2)));
    }
    aVector->FillSecondDerivatives();
  }
  delete ioni; delete ioniPC;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void EnergyLossForExtrapolator::ComputeTransportXS(const G4ParticleDefinition* part,
                                                   G4PhysicsTable* table)
{
  G4WentzelVIModel* msc = new G4WentzelVIModel();
  G4ParticleChange* mscPC = new G4ParticleChange();
  msc->SetParticleChange(mscPC);
  msc->SetPolarAngleLimit(CLHEP::pi);
  msc->Initialise(part, m_Cuts);

  m_Mass = part->GetPDGMass();
  double q = part->GetPDGCharge() / CLHEP::eplus;
  m_ChargeSq = q * q;
  m_Particle = part;

  const G4MaterialTable* mtable = G4Material::GetMaterialTable();
  B2DEBUG(1, "EnergyLossForExtrapolator::ComputeTransportXS for " << part->GetParticleName());

  for (G4int i = 0; i < m_NMaterials; i++) {

    const G4Material* mat = (*mtable)[i];
    msc->SetCurrentCouple(m_Couples[i]);
    B2DEBUG(10, "EnergyLossForExtrapolator::ComputeTransportXS(): i= " << i << "  mat= " << mat->GetName());
    G4PhysicsVector* aVector = (*table)[i];
    for (G4int j = 0; j <= m_Nbins; j++) {

      G4double e = aVector->Energy(j);
      G4double xs = msc->CrossSectionPerVolume(mat, part, e);
      aVector->PutValue(j, xs);
      B2DEBUG(10, "EnergyLossForExtrapolator::ComputeTransportXS(): j= " << j << "  e(MeV)= " << e / CLHEP::MeV
              << " xs(1/mm)= " << xs * CLHEP::mm);
    }
    aVector->FillSecondDerivatives();
  }
  delete msc; delete mscPC;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

