/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopoleEquation.h>

#include <globals.hh>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>
#include <iomanip>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

G4MonopoleEquation::G4MonopoleEquation(G4ElectroMagneticField* emField)
  : G4EquationOfMotion(emField)
{}

G4MonopoleEquation::~G4MonopoleEquation()
{}

void
G4MonopoleEquation::SetChargeMomentumMass(G4ChargeState particleChargeState,
                                          G4double      ,           // momentum,
                                          G4double particleMass)
{
  G4double particleMagneticCharge = particleChargeState.MagneticCharge();
  G4double particleElectricCharge = particleChargeState.GetCharge();

  //   fElCharge = particleElectricCharge;
  fElCharge = eplus * particleElectricCharge * c_light;

  fMagCharge =  eplus * particleMagneticCharge * c_light ;

  // G4cout << " G4MonopoleEquation: ElectricCharge=" << particleElectricCharge
  //           << "; MagneticCharge=" << particleMagneticCharge
  //           << G4endl;//TODO print with B2DEBUG or remove altogether

  fMassCof = particleMass * particleMass ;
}

void
G4MonopoleEquation::EvaluateRhsGivenB(const G4double y[],
                                      const G4double Field[],
                                      G4double dydx[]) const
{
  // Components of y:
  //    0-2 dr/ds,
  //    3-5 dp/ds - momentum derivatives

  G4double pSquared = y[3] * y[3] + y[4] * y[4] + y[5] * y[5] ;

  G4double Energy   = std::sqrt(pSquared + fMassCof);

  G4double pModuleInverse  = 1.0 / std::sqrt(pSquared);

  G4double inverse_velocity = Energy * pModuleInverse / c_light;

  G4double cofEl     = fElCharge * pModuleInverse ;
  G4double cofMag = fMagCharge * Energy * pModuleInverse;


  dydx[0] = y[3] * pModuleInverse ;
  dydx[1] = y[4] * pModuleInverse ;
  dydx[2] = y[5] * pModuleInverse ;

  // G4double magCharge = twopi * hbar_Planck / (eplus * mu0);
  // magnetic charge in SI units A*m convention
  //  see http://en.wikipedia.org/wiki/Magnetic_monopole
  //   G4cout  << "Magnetic charge:  " << magCharge << G4endl;
  // dp/ds = dp/dt * dt/ds = dp/dt / v = Force / velocity
  // dydx[3] = fMagCharge * Field[0]  * inverse_velocity  * c_light;
  // multiplied by c_light to convert to MeV/mm
  //     dydx[4] = fMagCharge * Field[1]  * inverse_velocity  * c_light;
  //     dydx[5] = fMagCharge * Field[2]  * inverse_velocity  * c_light;

  dydx[3] = cofMag * Field[0] + cofEl * (y[4] * Field[2] - y[5] * Field[1]);
  dydx[4] = cofMag * Field[1] + cofEl * (y[5] * Field[0] - y[3] * Field[2]);
  dydx[5] = cofMag * Field[2] + cofEl * (y[3] * Field[1] - y[4] * Field[0]);

  //        G4cout << std::setprecision(5)<< "E=" << Energy
  //               << "; p="<< 1/pModuleInverse
  //               << "; mC="<< magCharge
  //               <<"; x=" << y[0]
  //               <<"; y=" << y[1]
  //               <<"; z=" << y[2]
  //               <<"; dydx[3]=" << dydx[3]
  //               <<"; dydx[4]=" << dydx[4]
  //               <<"; dydx[5]=" << dydx[5]
  //               << G4endl;

  dydx[6] = 0.;//not used

  // Lab Time of flight
  dydx[7] = inverse_velocity;
  return;
}
