/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KAONPHYSICS_H
#define KAONPHYSICS_H

#include "G4VPhysicsConstructor.hh"

class G4TheoFSGenerator;
class G4FTFModel;
class G4ExcitedStringDecay;
class G4LundStringFragmentation;
class G4GeneratorPrecompoundInterface;

namespace Belle2 {

  namespace Simulation {

    /**
     * Kaon hadronic physics constructor for Belle II physics list
     */
    class KaonPhysics: public G4VPhysicsConstructor {
    public:
      KaonPhysics();
      ~KaonPhysics();

      virtual void ConstructParticle() override;
      virtual void ConstructProcess() override;

    private:
      G4TheoFSGenerator* m_ftfp;
      G4FTFModel* m_stringModel;
      G4ExcitedStringDecay* m_stringDecay;
      G4LundStringFragmentation* m_fragModel;
      G4GeneratorPrecompoundInterface* m_preCompoundModel;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // KAONPHYSICS_H
