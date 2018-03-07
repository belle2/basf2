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

#pragma once

#include <G4EquationOfMotion.hh>
#include <G4ElectroMagneticField.hh>

namespace Belle2 {

  namespace Monopoles {

    /**
     * Monopole equation of motion class.
     *
     * Equation of motion define stepper in G4, which defines chord finder,
     * which is required to perform transportation process.
     */
    class G4MonopoleEquation : public G4EquationOfMotion {
    public:

      /**
       * Constructor.
       *
       * @param emField Pointer to the field in which the particle will propagate.
       */
      explicit G4MonopoleEquation(G4ElectroMagneticField* emField);

      /**
       * Destructor.
       */
      ~G4MonopoleEquation();

      /**
       * G4EquationOfMotion::SetChargeMomentumMass() implementation.
       * Reads particle properties for equation of motion definition.
       *
       * @param particleChargeState Charge information about the particle.
       * Luckily, G4ChargeState have magnetic charge member variable.
       * Magnetic charge is taken from it in e+ untis.
       * @param momentum This argument of base function is ignored.
       * @param mass Mass of the particle.
       */
      virtual void  SetChargeMomentumMass(G4ChargeState particleChargeState,
                                          G4double      momentum,
                                          G4double      mass);

      /**
       * Given the value of the electromagnetic field, this function
       * calculates the value of the derivative dydx.
       *
       * @param y[6] 0-2 dr/ds - velocity, 3-5 dp/ds - momentum derivatives.
       * @param Field[3] Field components.
       * @param dydx[8] Where to store results.
       */
      virtual void EvaluateRhsGivenB(const G4double y[],
                                     const G4double Field[],
                                     G4double dydx[]) const;

    private:

      G4double  fMagCharge; /**< Magnetic charge of the monopole, in e+ units*/
      G4double  fElCharge; /**< Electric charge in case of a dyon*/
      G4double  fMassCof; /**< Square of the monopole mass*/
    };

  } //end Monopoles namespace

} //end Belle2 namespace
