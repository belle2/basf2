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
