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

#ifndef G4MONOPOLEPHYSICSMESSENGER_H
#define G4MONOPOLEPHYSICSMESSENGER_H

#include <globals.hh>
#include <G4UImessenger.hh>

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;

namespace Belle2 {

  namespace Monopoles {

    class G4MonopolePhysics;

    class G4MonopolePhysicsMessenger: public G4UImessenger {
    public:

      G4MonopolePhysicsMessenger(G4MonopolePhysics*);
      ~G4MonopolePhysicsMessenger();

      virtual void SetNewValue(G4UIcommand*, G4String);

    private:

      G4MonopolePhysics*         fPhys;

      G4UIdirectory*             fPhysicsDir;
      G4UIcommand*               fPhysicsCmd;
      G4UIcmdWithADouble*        fMCmd;
      G4UIcmdWithADouble*        fZCmd;
      G4UIcmdWithADoubleAndUnit* fMassCmd;

    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
