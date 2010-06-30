/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4MAGNETICFIELDMESSENGER_H
#define B4MAGNETICFIELDMESSENGER_H

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"

namespace Belle2 {

  class B4MagneticField;

//! Class for MagneticField Messenger
  /*! This class is used to set new values for variables defined in class B4MagneticField.
      These new values will read from macro file, so if you want to change some values,
      don't need to re-compile the program, just edit the macro.
  */

  class B4MagneticFieldMessenger: public G4UImessenger {

  public:

    //! Constructor
    B4MagneticFieldMessenger(B4MagneticField* fieldSetup);

    //! Destructor
    ~B4MagneticFieldMessenger();

    //! The method to set new values
    /*! Set new value for each variable defined in class B4MagneticField.
        \param command A pointer of G4UIcommand is used to set/get new values with different types.
        \param newvalue A string contains new values.
    */
    void SetNewValue(G4UIcommand* command, G4String newvalue);

  private:

    B4MagneticField* m_fieldSetup;
    G4UIdirectory* m_fieldDir;
    G4UIcommand* m_fieldCmd;

  };

} // end of Belle2 namespace

#endif // B4MAGNETICFIELDMESSENGER_H
