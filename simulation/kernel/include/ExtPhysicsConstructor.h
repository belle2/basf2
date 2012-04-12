/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTPHYSICSCONSTRUCTOR_H_
#define EXTPHYSICSCONSTRUCTOR_H_

#include <globals.hh>
#include <G4VPhysicsConstructor.hh>

namespace Belle2 {

  namespace Simulation {

    /**
     * Define geant4e-specific physics
     */
    class ExtPhysicsConstructor: public G4VPhysicsConstructor {

    public:

      /** The ExtPhysicsConstructor constructor */
      ExtPhysicsConstructor();

      /** The ExtPhysicsConstructor destructor */
      virtual ~ExtPhysicsConstructor();

      /** ConstructParticle() defines the geant4e-specific particles */
      virtual void ConstructParticle();

      /** ConstructProcess() defines the geant4e-specific processes */
      virtual void ConstructProcess();

    private:

    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTPHYSICSCONSTRUCTOR_H_ */
