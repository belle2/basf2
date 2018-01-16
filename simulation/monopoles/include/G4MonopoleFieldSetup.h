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

#ifndef G4MONOPOLEFIELDSETUP_H
#define G4MONOPOLEFIELDSETUP_H

#include <G4MagneticField.hh>
#include <G4UniformMagField.hh>

class G4FieldManager;
class G4ChordFinder;
class G4Mag_UsualEqRhs;
class G4MagIntegratorStepper;

namespace Belle2 {

  namespace Monopoles {

    class G4MonopoleFieldMessenger;
    class G4MonopoleEquation;

    class G4MonopoleFieldSetup {
    public:

      void InitialiseAll();    //  Set parameters and call method below
      void SetMagField(G4double fieldValue);
      void SetStepperAndChordFinder(G4int val);

      static G4MonopoleFieldSetup* GetMonopoleFieldSetup();

      ~G4MonopoleFieldSetup() ;

    private:

      G4MonopoleFieldSetup();

      G4FieldManager*         GetGlobalFieldManager() ;   // static

      G4FieldManager*         fFieldManager ;
      G4ChordFinder*          fChordFinder ;
      G4Mag_UsualEqRhs*       fEquation ;
      G4MonopoleEquation*     fMonopoleEquation ;
      G4MagneticField*        fMagneticField ;

      G4MagIntegratorStepper* fStepper ;
      G4MagIntegratorStepper* fMonopoleStepper ;

      G4double                fMinStep ;

      static G4MonopoleFieldSetup*  fMonopoleFieldSetup;
      G4MonopoleFieldMessenger*     fMonopoleFieldMessenger;

    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
