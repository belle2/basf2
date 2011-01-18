/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Andreas Moll                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PHYSICSLIST_H_
#define PHYSICSLIST_H_

#include <globals.hh>
#include <G4VModularPhysicsList.hh>
#include <CompileTimeConstraints.hh>
#include <G4UnknownParticle.hh>
#include <G4UnknownDecay.hh>
#include <G4ProcessManager.hh>

namespace Belle2 {

  namespace Simulation {

    /**
     * The basf2 physics list.
     * Takes an existing physics list as template and adds the
     * G4UnknownParticle to it.
     */
    template<class T> class PhysicsList: public T {

    public:

      /**
       * The PhysicsList constructor.
       * @param ver The verbosity level of the physics list.
       */
      PhysicsList(G4int ver = 1) : T(ver) {}

      /**
       * Instantiates each particle type.
       */
      virtual void ConstructParticle() {
        T::ConstructParticle();
        G4UnknownParticle::UnknownParticleDefinition();
      }

      /**
       * Instantiates each physics process and registers it to the process manager of each particle type.
       */
      virtual void ConstructProcess() {
        T::ConstructProcess();
        G4ParticleDefinition* unknown = G4UnknownParticle::UnknownParticleDefinition();
        G4ProcessManager* pmanager = unknown->GetProcessManager();
        pmanager->AddProcess(&m_UnknownDecay);
        pmanager->SetProcessOrdering(&m_UnknownDecay, idxPostStep);
      }


    private:

      G4UnknownDecay m_UnknownDecay; /**< Object of an unknown decay. */

      /**Enum to check for a valid class hierarchy at compile time. */
      enum {ok = CompileTimeConstraints::IsA<T, G4VModularPhysicsList>::ok };
    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* PHYSICSLIST_H_ */
