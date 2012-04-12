/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTNOHITS_H_
#define EXTNOHITS_H_

#include <globals.hh>
#include <G4VDiscreteProcess.hh>

class G4ParticleDefinition;
class G4Track;
class G4Step;

namespace Belle2 {

  namespace Simulation {

    /**
     * Define physics process that avoids hit invocation in sensitive volumes
     */
    class ExtNoHits: public G4VDiscreteProcess {

    public:

      //! constructor
      ExtNoHits(const G4String& processName = "ExtNoHits", G4ProcessType aType = fNotDefined);

      //! destructor
      virtual ~ExtNoHits();

      //! Reports if this process is applicable to the particle being swum
      inline G4bool IsApplicable(const G4ParticleDefinition&);

      //! Sets flag at end of each step to avoid hit invocation
      G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&);

      //! Gets physical interaction length of this process at end of each step
      inline G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*);

      //! Gets mean free path of this process at end of each step
      inline G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*);

    private:

      //! Hide assignment operator
      ExtNoHits(const ExtNoHits&);

      //! Hide assignment operator
      ExtNoHits& operator = (const ExtNoHits&);

      //! Pointer to a reusable G4ParticleChange object
      G4ParticleChange* m_particleChange;

    };

    inline G4bool ExtNoHits::IsApplicable(const G4ParticleDefinition&)
    {
      return true; // this process applies to all particles
    }

    inline G4double ExtNoHits::PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition* condition)
    {
      *condition = Forced;
      return kInfinity;
    }

    inline G4double ExtNoHits::GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*)
    {
      return kInfinity;
    }

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTNOHITS_H_ */
