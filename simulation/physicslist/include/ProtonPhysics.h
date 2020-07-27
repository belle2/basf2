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

      /** Build processes, models, cross sections used in physics list */
      virtual void ConstructProcess() override;

      /** Build all particle types used in physics list (empty in this class) */
      virtual void ConstructParticle() override;

    private:
      /** Final state generator for QCD string models */
      G4TheoFSGenerator* m_ftfp;

      /** Fritiof string model */
      G4FTFModel* m_stringModel;

      /** Model to decay strings into hadrons */
      G4ExcitedStringDecay* m_stringDecay;

      /** Lund string fragmentation model */
      G4LundStringFragmentation* m_fragModel;

      /** Precompound model to deexcite post-collision nucleus */
      G4GeneratorPrecompoundInterface* m_preCompoundModel;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // PROTONPHYSICS_H
