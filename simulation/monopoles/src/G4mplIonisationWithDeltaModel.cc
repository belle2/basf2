/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// References
// [1] Steven P. Ahlen: Energy loss of relativistic heavy ionizing particles,
//     S.P. Ahlen, Rev. Mod. Phys 52(1980), p121
// [2] K.A. Milton arXiv:hep-ex/0602040
// [3] S.P. Ahlen and K. Kinoshita, Phys. Rev. D26 (1982) 2347

// modified from GEANT4 exoticphysics/monopole/*
// works only for low magnetic charge, higher charge corrections are not used

#include <simulation/monopoles/G4mplIonisationWithDeltaModel.h>

#include <Randomize.hh>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>
#include <G4ParticleChangeForLoss.hh>
#include <G4Electron.hh>
#include <G4DynamicParticle.hh>
#include <G4ProductionCutsTable.hh>
#include <G4MaterialCutsCouple.hh>
#include <G4Log.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

std::vector<G4double>* G4mplIonisationWithDeltaModel::dedx0 = nullptr;

G4mplIonisationWithDeltaModel::G4mplIonisationWithDeltaModel(G4double mCharge,
    const G4String& nam)
  : G4VEmModel(nam), G4VEmFluctuationModel(nam),
    magCharge(mCharge),
    twoln10(log(100.0)),
    betalow(0.01),
    betalim(0.1),
    beta2lim(betalim * betalim),
    bg2lim(beta2lim * (1.0 + beta2lim))
{
  pi_hbarc2_over_mc2 = pi * hbarc * hbarc / electron_mass_c2;
  chargeSquare = magCharge * magCharge * 4 * fine_structure_const *
                 fine_structure_const; //Formulas below assume Dirac charge units for magnetic charge, g_D = 68.5e
  dedxlim = 45. * chargeSquare * GeV * cm2 / g;
  fParticleChange = nullptr;
  theElectron = G4Electron::Electron();
  G4cout << "### Monopole ionisation model with d-electron production, Gmag= "
         << magCharge / eplus << G4endl;//TODO print it with B2INFO
  monopole = nullptr;
  mass = 0.0;
}

G4mplIonisationWithDeltaModel::~G4mplIonisationWithDeltaModel()
{
  if (IsMaster()) { delete dedx0; }
}

void G4mplIonisationWithDeltaModel::SetParticle(const G4ParticleDefinition* p)
{
  monopole = p;
  mass     = monopole->GetPDGMass();
  G4double emin =
    std::min(LowEnergyLimit(), 0.1 * mass * (1. / sqrt(1. - betalow * betalow) - 1.));
  G4double emax =
    std::max(HighEnergyLimit(), 10 * mass * (1. / sqrt(1. - beta2lim) - 1.));
  SetLowEnergyLimit(emin);
  SetHighEnergyLimit(emax);
}

void
G4mplIonisationWithDeltaModel::Initialise(const G4ParticleDefinition* p,
                                          const G4DataVector&)
{
  if (!monopole) { SetParticle(p); }
  if (!fParticleChange) { fParticleChange = GetParticleChangeForLoss(); }
  if (IsMaster()) {
    if (!dedx0) { dedx0 = new std::vector<G4double>; }
    G4ProductionCutsTable* theCoupleTable =
      G4ProductionCutsTable::GetProductionCutsTable();
    G4int numOfCouples = theCoupleTable->GetTableSize();
    G4int n = dedx0->size();
    if (n < numOfCouples) { dedx0->resize(numOfCouples); }

    // initialise vector
    for (G4int i = 0; i < numOfCouples; ++i) {

      const G4Material* material =
        theCoupleTable->GetMaterialCutsCouple(i)->GetMaterial();
      G4double eDensity = material->GetElectronDensity();
      G4double vF = electron_Compton_length * pow(3.*pi * pi * eDensity, 0.3333333333);
      (*dedx0)[i] = pi_hbarc2_over_mc2 * eDensity * chargeSquare *
                    (G4Log(2 * vF / fine_structure_const) - 0.5) / vF;
    }
  }
}

G4double
G4mplIonisationWithDeltaModel::ComputeDEDXPerVolume(const G4Material* material,
                                                    const G4ParticleDefinition* p,
                                                    G4double kineticEnergy,
                                                    G4double maxEnergy)
{
  if (!monopole) { SetParticle(p); }
  G4double tmax = MaxSecondaryEnergy(p, kineticEnergy);
  G4double cutEnergy = std::min(tmax, maxEnergy);
  cutEnergy = std::max(LowEnergyLimit(), cutEnergy);
  G4double tau   = kineticEnergy / mass;
  G4double gam   = tau + 1.0;
  G4double bg2   = tau * (tau + 2.0);
  G4double beta2 = bg2 / (gam * gam);
  G4double beta  = sqrt(beta2);

  // low-energy asymptotic formula
  //G4double dedx  = dedxlim*beta*material->GetDensity();
  G4double dedx = (*dedx0)[CurrentCouple()->GetIndex()] * beta;

  // above asymptotic
  if (beta > betalow) {

    // high energy
    if (beta >= betalim) {
      dedx = ComputeDEDXAhlen(material, bg2, cutEnergy);

    } else {

      //G4double dedx1 = dedxlim*betalow*material->GetDensity();
      G4double dedx1 = (*dedx0)[CurrentCouple()->GetIndex()] * betalow;
      G4double dedx2 = ComputeDEDXAhlen(material, bg2lim, cutEnergy);

      // extrapolation between two formula
      G4double kapa2 = beta - betalow;
      G4double kapa1 = betalim - beta;
      dedx = (kapa1 * dedx1 + kapa2 * dedx2) / (kapa1 + kapa2);
    }
  }
  return dedx;
}

G4double
G4mplIonisationWithDeltaModel::ComputeDEDXAhlen(const G4Material* material,
                                                G4double bg2,
                                                G4double cutEnergy)
{
  G4double eDensity = material->GetElectronDensity();
  G4double eexc  = material->GetIonisation()->GetMeanExcitationEnergy();

  // Ahlen's formula for nonconductors, [1]p157, f(5.7)
  G4double dedx =
    0.5 * (log(2.0 * electron_mass_c2 * bg2 * cutEnergy / (eexc * eexc)) - 1.0);//"Conventional" ionisation
//   G4double dedx =
//     1.0 * (log(2.0 * electron_mass_c2 * bg2 * cutEnergy / (eexc * eexc)));//Fryberger magneticon double ionisation

  // density effect correction
  G4double x = G4Log(bg2) / twoln10;
  dedx -= material->GetIonisation()->DensityCorrection(x);

  // now compute the total ionization loss
  dedx *=  pi_hbarc2_over_mc2 * eDensity * chargeSquare;

  if (dedx < 0.0) { dedx = 0.; }
  return dedx;
}

G4double
G4mplIonisationWithDeltaModel::ComputeCrossSectionPerElectron(
  const G4ParticleDefinition* p,
  G4double kineticEnergy,
  G4double cut,
  G4double maxKinEnergy)
{
  if (!monopole) { SetParticle(p); }
  G4double cross = 0.0;
  G4double tmax = MaxSecondaryEnergy(p, kineticEnergy);
  G4double maxEnergy = std::min(tmax, maxKinEnergy);
  G4double cutEnergy = std::max(LowEnergyLimit(), cut);
  if (cutEnergy < maxEnergy) {
    cross = (0.5 / cutEnergy - 0.5 / maxEnergy) * pi_hbarc2_over_mc2 * chargeSquare;
  }
  return cross;
}

G4double
G4mplIonisationWithDeltaModel::ComputeCrossSectionPerAtom(
  const G4ParticleDefinition* p,
  G4double kineticEnergy,
  G4double Z, G4double,
  G4double cutEnergy,
  G4double maxEnergy)
{
  G4double cross =
    Z * ComputeCrossSectionPerElectron(p, kineticEnergy, cutEnergy, maxEnergy);
  return cross;
}

void
G4mplIonisationWithDeltaModel::SampleSecondaries(vector<G4DynamicParticle*>* vdp,
                                                 const G4MaterialCutsCouple*,
                                                 const G4DynamicParticle* dp,
                                                 G4double minKinEnergy,
                                                 G4double maxEnergy)
{
  G4double kineticEnergy = dp->GetKineticEnergy();
  G4double tmax = MaxSecondaryEnergy(dp->GetDefinition(), kineticEnergy);

  G4double maxKinEnergy = std::min(maxEnergy, tmax);
  if (minKinEnergy >= maxKinEnergy) { return; }

  //G4cout << "G4mplIonisationWithDeltaModel::SampleSecondaries: E(GeV)= "
  //   << kineticEnergy/GeV << " M(GeV)= " << mass/GeV
  //   << " tmin(MeV)= " << minKinEnergy/MeV << G4endl;//TODO print with B2DEBUG or remove altogether

  G4double totEnergy     = kineticEnergy + mass;
  G4double etot2         = totEnergy * totEnergy;
  G4double beta2         = kineticEnergy * (kineticEnergy + 2.0 * mass) / etot2;

  // sampling without nuclear size effect
  G4double q = G4UniformRand();
  G4double deltaKinEnergy = minKinEnergy * maxKinEnergy
                            / (minKinEnergy * (1.0 - q) + maxKinEnergy * q);

  // delta-electron is produced
  G4double totMomentum = totEnergy * sqrt(beta2);
  G4double deltaMomentum =
    sqrt(deltaKinEnergy * (deltaKinEnergy + 2.0 * electron_mass_c2));
  G4double cost = deltaKinEnergy * (totEnergy + electron_mass_c2) /
                  (deltaMomentum * totMomentum);
  if (cost > 1.0) { cost = 1.0; }

  G4double sint = sqrt((1.0 - cost) * (1.0 + cost));

  G4double phi = twopi * G4UniformRand() ;

  G4ThreeVector deltaDirection(sint * cos(phi), sint * sin(phi), cost);
  G4ThreeVector direction = dp->GetMomentumDirection();
  deltaDirection.rotateUz(direction);

  // create G4DynamicParticle object for delta ray
  G4DynamicParticle* delta =
    new G4DynamicParticle(theElectron, deltaDirection, deltaKinEnergy);

  vdp->push_back(delta);

  // Change kinematics of primary particle
  kineticEnergy       -= deltaKinEnergy;
  G4ThreeVector finalP = direction * totMomentum - deltaDirection * deltaMomentum;
  finalP               = finalP.unit();

  fParticleChange->SetProposedKineticEnergy(kineticEnergy);
  fParticleChange->SetProposedMomentumDirection(finalP);
}

G4double G4mplIonisationWithDeltaModel::SampleFluctuations(
  const G4MaterialCutsCouple* couple,
  const G4DynamicParticle* dp,
  G4double tmax,
  G4double length,
  G4double meanLoss)
{
  G4double siga = Dispersion(couple->GetMaterial(), dp, tmax, length);
  G4double loss = meanLoss;
  siga = sqrt(siga);
  G4double twomeanLoss = meanLoss + meanLoss;

  if (twomeanLoss < siga) {
    G4double x;
    do {
      loss = twomeanLoss * G4UniformRand();
      x = (loss - meanLoss) / siga;
      // Loop checking, 07-Aug-2015, Vladimir Ivanchenko
    } while (1.0 - 0.5 * x * x < G4UniformRand());
  } else {
    do {
      loss = G4RandGauss::shoot(meanLoss, siga);
      // Loop checking, 07-Aug-2015, Vladimir Ivanchenko
    } while (0.0 > loss || loss > twomeanLoss);
  }
  return loss;
}

G4double
G4mplIonisationWithDeltaModel::Dispersion(const G4Material* material,
                                          const G4DynamicParticle* dp,
                                          G4double tmax,
                                          G4double length)
{
  G4double siga = 0.0;
  G4double tau   = dp->GetKineticEnergy() / mass;
  if (tau > 0.0) {
    G4double electronDensity = material->GetElectronDensity();
    G4double gam   = tau + 1.0;
    G4double invbeta2 = (gam * gam) / (tau * (tau + 2.0));
    siga  = (invbeta2 - 0.5) * twopi_mc2_rcl2 * tmax * length
            * electronDensity * chargeSquare;
  }
  return siga;
}

G4double
G4mplIonisationWithDeltaModel::MaxSecondaryEnergy(const G4ParticleDefinition*,
                                                  G4double kinEnergy)
{
  G4double tau = kinEnergy / mass;
  return 2.0 * electron_mass_c2 * tau * (tau + 2.);
}
