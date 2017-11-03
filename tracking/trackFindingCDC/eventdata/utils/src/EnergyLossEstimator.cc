/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/utils/EnergyLossEstimator.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>

#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <geometry/GeometryManager.h>

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include "G4ThreeVector.hh"
#include "G4Navigator.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Material.hh"

using namespace Belle2;
using namespace TrackFindingCDC;

EnergyLossEstimator EnergyLossEstimator::forCDC()
{

  // Look up material properties from the Belle2 geometry
  G4Navigator g4Nav;
  G4VPhysicalVolume* g4World = geometry::GeometryManager::getInstance().getTopVolume();
  g4Nav.SetWorldVolume(g4World);

  // Choose position well inside the CDC
  double posX = 0;
  double posY = 50;
  double posZ = 0;

  G4ThreeVector g4Pos(posX * CLHEP::cm, posY * CLHEP::cm, posZ * CLHEP::cm);
  const G4VPhysicalVolume* g4Volume = g4Nav.LocateGlobalPointAndSetup(g4Pos);
  const G4Material* g4Mat = g4Volume->GetLogicalVolume()->GetMaterial();

  double A = 0;
  double Z = 0;

  if (g4Mat->GetNumberOfElements() == 1) {
    A = g4Mat->GetA() * CLHEP::mole / CLHEP::g;
    Z = g4Mat->GetZ();
  } else {
    // Calculate weight-averaged A, Z
    for (unsigned i = 0; i < g4Mat->GetNumberOfElements(); ++i) {
      const G4Element* element = (*g4Mat->GetElementVector())[i];
      const double elementA = element->GetA() * CLHEP::mole / CLHEP::g;
      const double elementZ = element->GetZ();
      const double frac = g4Mat->GetFractionVector()[i];
      B2RESULT("Part " << i << " Z=" << elementZ << " A=" << elementA << " (" << frac << ")");
      Z += elementZ * frac;
      A += elementA * frac;
    }
  }

  // Make sure to translate to Belle units from the Geant4 / CLHEP units
  // Z has correct units (is unitless)
  const double density = g4Mat->GetDensity() / CLHEP::g * CLHEP::cm3;
  const double radiationLength = g4Mat->GetRadlen() / CLHEP::cm;
  const double mEE = g4Mat->GetIonisation()->GetMeanExcitationEnergy() / CLHEP::GeV;

  B2RESULT("Received Z " << Z);
  B2RESULT("Received A " << A);
  B2RESULT("Received density " << density);
  B2RESULT("Received radiation length " << radiationLength);
  B2RESULT("Received mean excitation energy " << mEE);

  const double eDensity = Z * density / A;
  B2RESULT("Received electron density " << eDensity);

  const double bZ = CDCBFieldUtil::getBFieldZ();

  return EnergyLossEstimator(eDensity, mEE, bZ);
}

double EnergyLossEstimator::getMass(int pdgCode)
{
  if (std::abs(pdgCode) == 11) {
    const double electronMass = 0.511 * Unit::MeV;
    return electronMass;
  } else if (std::abs(pdgCode) == 13) {
    const double muonMass = 195.658 * Unit::MeV;
    return muonMass;
  } else if (std::abs(pdgCode) == 321) {
    const double kaonMass = 0.493677;
    return kaonMass;
  } else if (std::abs(pdgCode) == 211) {
    const double pionMass = 0.13957;
    return pionMass;
  } else if (std::abs(pdgCode) == 2212) {
    const double protonMass = 0.938272;
    return protonMass;
  }
  return NAN;
}

int EnergyLossEstimator::getCharge(int pdgCode)
{
  if (std::abs(pdgCode) == 11) {
    return -sign(pdgCode);
  } else if (std::abs(pdgCode) == 13) {
    return -sign(pdgCode);
  } else if (std::abs(pdgCode) == 321) {
    return sign(pdgCode);
  } else if (std::abs(pdgCode) == 211) {
    return sign(pdgCode);
  } else if (std::abs(pdgCode) == 2212) {
    return sign(pdgCode);
  }
  return 0;
}

EnergyLossEstimator::EnergyLossEstimator(double eDensity, double I, double bZ)
  : m_eDensity(eDensity)
  , m_I(I)
  , m_bZ(bZ)
{
}

double EnergyLossEstimator::getBetheStoppingPower(double p, int pdgCode) const
{
  static const double eMass = getMass(11);

  const double M = getMass(pdgCode);

  const double E = std::sqrt(p * p + M * M);
  const double gamma = E / M;
  const double beta = p / E;

  const double beta2 = beta * beta;
  const double gamma2 = gamma * gamma;

  const double Wmax = 2 * eMass * beta2 * gamma2 / (1 + 2 * gamma * eMass / M);
  const double I2 =  m_I * m_I;

  static const double K = 0.307075 * Unit::MeV * Unit::cm2;
  const double dEdx = K * m_eDensity / beta2 *
                      (1.0 / 2.0 * std::log(2 * eMass * beta2 * gamma2 * Wmax / I2) - beta2);
  return dEdx;
}

double EnergyLossEstimator::getEnergyLoss(double p, int pdgCode, double arcLength) const
{
  const double dEdx = getBetheStoppingPower(p, pdgCode);
  const double eLoss = arcLength * dEdx;
  return eLoss;
}

double EnergyLossEstimator::getMomentumLossFactor(double p, int pdgCode, double arcLength) const
{
  const double eLoss = getEnergyLoss(p, pdgCode, arcLength);
  return (p - eLoss) / p;

  const double mass = getMass(pdgCode);

  const double eBefore = std::sqrt(p * p + mass * mass);
  const double eAfter = eBefore - eLoss;
  const double pAfter = std::sqrt(eAfter * eAfter - mass * mass);
  return pAfter / p;
}

double EnergyLossEstimator::getLossDist2D(double pt, int pdgCode, double arcLength2D) const
{
  const double eLoss = getEnergyLoss(pt, pdgCode, arcLength2D);

  const int q = getCharge(pdgCode);
  const double radius = q * CDCBFieldUtil::absMom2DToBendRadius(pt, m_bZ);;
  return radius * eLoss / (pt - eLoss);

  // const double pFactor = getMomentumLossFactor(p, pdgCode, arcLength2D);
  // return radius * (1 / pFactor - 1);
}
