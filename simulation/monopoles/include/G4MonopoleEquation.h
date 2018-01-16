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

#ifndef G4MONOPOLEEQUATION_H
#define G4MONOPOLEEQUATION_H

#include <G4EquationOfMotion.hh>
#include <G4ElectroMagneticField.hh>

namespace Belle2 {

  namespace Monopoles {

    class G4MonopoleEquation : public G4EquationOfMotion {
    public:  // with description

      G4MonopoleEquation(G4ElectroMagneticField* emField);

      ~G4MonopoleEquation();

      virtual void  SetChargeMomentumMass(G4ChargeState particleChargeState,
                                          G4double      momentum,
                                          G4double      mass);
      // magnetic charge in e+ units

      virtual void EvaluateRhsGivenB(const G4double y[],
                                     const G4double Field[],
                                     G4double dydx[]) const;
      // Given the value of the electromagnetic field, this function
      // calculates the value of the derivative dydx.

    private:

      G4double  fMagCharge;
      G4double  fElCharge;
      G4double  fMassCof;
    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
