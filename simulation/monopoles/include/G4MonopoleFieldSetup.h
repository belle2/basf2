//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// modified from exoticphysics/monopole/*

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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
