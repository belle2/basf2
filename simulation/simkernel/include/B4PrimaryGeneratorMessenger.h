/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4PRIMARYGENERATORMESSENGER_H
#define B4PRIMARYGENERATORMESSENGER_H

#include <simkernel/B4PrimaryGeneratorAction.h>

#include "G4UImessenger.hh"
#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4ParticleTable.hh"

namespace Belle2 {

  class B4PrimaryGeneratorMessenger: public G4UImessenger {

  public:

    //! Constructor
    /*!
        \param mpga The pointer of B4PrimaryGeneratorAction
    */
    B4PrimaryGeneratorMessenger(Belle2::B4PrimaryGeneratorAction* mpga);

    //! Destructor
    ~B4PrimaryGeneratorMessenger();

    //! The method to set new values for member variables in class B4PrimaryGeneratorAction
    /*!
        \param command The pointer of G4 user interface to set parameters.
        \param newValues A string contains all parameters set by the pointer of G4UIcommand.
    */
    void SetNewValue(G4UIcommand * command, G4String newValues);

    //! The method to get current value, but not implemented now
    /*!
        \param command The pointer of G4 user interface to get parameters.
        \return A string that contains all parameters set by pointer command.
    */
    G4String GetCurrentValue(G4UIcommand * command);

  private:

    B4PrimaryGeneratorAction* m_myAction;            /*!< The pointer that points class B4PrimaryGeneratorAction. */
    G4ParticleTable* m_particleTable;                /*!< The pointer that points class  G4ParticleTable. */

    G4UIdirectory* m_generatorDirectory;             /*!< The pointer that points class G4UIdirectory. */
    G4UIcmdWithAString* m_gennamCmd;                 /*!< The pointer of a command with string type to set generator name. */
    G4UIcmdWithAString* m_HEPEvtCmd;                 /*!< The pointer of a command with string type to set HEPEvt parameters. */
    G4UIcommand* m_testerCmd;                        /*!< The pointer that points class G4UIcommand. */
  };

} // end namespace Belle2

#endif /* B4PRIMARYGENERATORMESSENGER_H */
