/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PROTONPHYSICS_H
#define PROTONPHYSICS_H

#include "G4VPhysicsConstructor.hh"

class G4TheoFSGenerator;
class G4FTFModel;
class G4ExcitedStringDecay;
class G4LundStringFragmentation;
class G4GeneratorPrecompoundInterface;

namespace Belle2 {

  namespace Simulation {

    /**
     * Proton hadronic physics constructor for Belle II physics list
     */
    class ProtonPhysics: public G4VPhysicsConstructor {
    public:
      ProtonPhysics();
      ~ProtonPhysics();

      virtual void ConstructProcess() override;
      virtual void ConstructParticle() override;

    private:
      G4TheoFSGenerator* m_ftfp;
      G4FTFModel* m_stringModel;
      G4ExcitedStringDecay* m_stringDecay;
      G4LundStringFragmentation* m_fragModel;
      G4GeneratorPrecompoundInterface* m_preCompoundModel;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // PROTONPHYSICS_H
