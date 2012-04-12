/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTMAGFIELDLIMITPROCESS_H_
#define EXTMAGFIELDLIMITPROCESS_H_

#include <globals.hh>
#include <G4VErrorLimitProcess.hh>

class G4Track;
class G4Field;

namespace Belle2 {

  namespace Simulation {

    //! Defines the physics process that avoids hit invocation in sensitive volumes
    class ExtMagFieldLimitProcess: public G4VErrorLimitProcess {

    public:

      //! constructor
      ExtMagFieldLimitProcess(const G4String& processName = "ExtMagFieldLimitProcess");

      //! destructor
      ~ExtMagFieldLimitProcess();

      //! Returns the step length after each step
      virtual G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*);

    private:

      //! Stores the pointer to the magnetic field class
      const G4Field* m_field;

      //! Stores the step limit for this process
      G4double m_stepLimit;
    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTMAGFIELDLIMITPROCESS_H_ */
