/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#ifndef G4MONOPOLEFIELDMESSENGER_H
#define G4MONOPOLEFIELDMESSENGER_H

#include <globals.hh>
#include <G4UImessenger.hh>

class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;

namespace Belle2 {

  namespace Monopoles {

    class G4MonopoleFieldSetup;

    class G4MonopoleFieldMessenger: public G4UImessenger {
    public:

      G4MonopoleFieldMessenger(G4MonopoleFieldSetup*);
      ~G4MonopoleFieldMessenger();

      virtual void SetNewValue(G4UIcommand*, G4String);

    private:

      G4MonopoleFieldSetup*  fField;

      G4UIdirectory*             fFieldDir;
      G4UIcmdWithADoubleAndUnit* fSetFieldCmd;
    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
