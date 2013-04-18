/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTSTEPLENGTHLIMITPROCESS_H_
#define EXTSTEPLENGTHLIMITPROCESS_H_

#include <globals.hh>
#include "G4VDiscreteProcess.hh"

class G4Track;
class G4Field;

namespace Belle2 {

  namespace Simulation {

    //! Defines the physics process that avoids hit invocation in sensitive volumes
    class ExtStepLengthLimitProcess: public G4VDiscreteProcess {

    public:

      //! constructor
      ExtStepLengthLimitProcess(const G4String& processName = "ExtStepLengthLimitProcess");

      //! destructor
      ~ExtStepLengthLimitProcess();

      //! Do nothing special after the particle has stepped
      G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&);

      //! Returns the step length after each step
      G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*);

      //! Returns the mean free path (always infinity!) after each step
      G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*);

      //! Returns the step limit
      G4double GetStepLimit() const { return m_stepLimit; }

      //! Sets the step limit
      void SetStepLimit(G4double val) { m_stepLimit = val; }

    private:

      //! Pointer to a reusable G4ParticleChange object
      G4ParticleChange* m_particleChange;

    protected:

      //! Stores the step limit
      G4double m_stepLimit;

    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTSTEPLENGTHLIMITPROCESS_H_ */
