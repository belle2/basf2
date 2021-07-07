/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef EXTSTEPLENGTHLIMITPROCESS_H_
#define EXTSTEPLENGTHLIMITPROCESS_H_

#include <globals.hh>
#include "G4VDiscreteProcess.hh"

class G4Track;

namespace Belle2 {

  namespace Simulation {

    //! Defines the physics process that avoids hit invocation in sensitive volumes
    class ExtStepLengthLimitProcess: public G4VDiscreteProcess {

    public:

      //! constructor
      explicit ExtStepLengthLimitProcess(const G4String& processName = "ExtStepLengthLimitProcess");

      //! destructor
      ~ExtStepLengthLimitProcess();

      //! See if the particle has left the geant4e "target" volume
      G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&);

      //! Returns the step length after each step
      G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*);

      //! Returns the mean free path for this process
      G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*);

      //! Returns the step limit
      G4double GetStepLimit() const { return m_stepLimit; }

      //! Sets the step limit
      void SetStepLimit(G4double val) { m_stepLimit = val; }

    protected:

      //! Stores the step limit
      G4double m_stepLimit;

    };

    inline G4double ExtStepLengthLimitProcess::PostStepGetPhysicalInteractionLength(const G4Track&, G4double,
        G4ForceCondition* condition)
    {
      *condition = Forced; // PostStepDoIt() must be called after each step
      return m_stepLimit;
    }

    inline G4double ExtStepLengthLimitProcess::GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*)
    {
      return kInfinity;
    }


  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTSTEPLENGTHLIMITPROCESS_H_ */
