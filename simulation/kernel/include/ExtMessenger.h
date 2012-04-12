/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTMESSENGER_H_
#define EXTMESSENGER_H_

#include <globals.hh>
#include <G4UImessenger.hh>

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;
class G4ErrorStepLengthLimitProcess;

namespace Belle2 {

  namespace Simulation {

    class ExtMagFieldLimitProcess;
    class ExtEnergyLoss;

    /**
     * Define the mechanism by which the user can communicate with
     * the geant4e extrapolator using geant4 UI commands
     */
    class ExtMessenger: public G4UImessenger {

    public:

      //! constructor
      ExtMessenger(G4ErrorStepLengthLimitProcess*,
                   ExtMagFieldLimitProcess*,
                   ExtEnergyLoss*);

      //! destructor
      ~ExtMessenger();

      //! Process a geant4 UI command
      void SetNewValue(G4UIcommand*, G4String);

    private:

      //! Process that limits the geant4e step length
      G4ErrorStepLengthLimitProcess* m_stepLengthAction;

      //! Process that limits the geant4e step length in magnetic field
      ExtMagFieldLimitProcess* m_magFieldAction;

      //! Process that limits the geant4e step length due to energy loss
      ExtEnergyLoss* m_energyLossAction;

      //! Defines UI directory for "/geant4e/" commands
      G4UIdirectory* m_dir;

      //! Defines UI directory for "/geant4e/limits/" commands
      G4UIdirectory* m_dirLimits;

      //! Defines UI command "/geant4e/limits/stepLength" to limit step length
      G4UIcmdWithADoubleAndUnit* m_stepLengthLimitCmd;

      //! Defines UI command "/geant4e/limits/magField" to limit step length
      G4UIcmdWithADouble* m_magFieldLimitCmd;

      //! defines UI command "/geant4e/limits/energyLoss" to limit step length
      G4UIcmdWithADouble* m_energyLossCmd;

    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTMESSENGER_H_ */
