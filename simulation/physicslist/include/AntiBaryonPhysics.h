/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ANTIBARYONPHYSICS_H
#define ANTIBARYONPHYSICS_H

#include "G4VPhysicsConstructor.hh"

class G4TheoFSGenerator;
class G4FTFModel;
class G4ExcitedStringDecay;
class G4LundStringFragmentation;
class G4GeneratorPrecompoundInterface;
class G4ComponentAntiNuclNuclearXS;

namespace Belle2 {

  namespace Simulation {

    /**
     * Anti-baryon hadronic physics constructor for Belle II physics list
     */
    class AntiBaryonPhysics: public G4VPhysicsConstructor {
    public:
      AntiBaryonPhysics();
      ~AntiBaryonPhysics();

      /** Build processes, models, cross sections used in physics list */
      virtual void ConstructParticle() override;

      /** Build all particle types used in physics list (empty in this class) */
      virtual void ConstructProcess() override;

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

      /** Cross section set for anti-nucleon nucleus inelastic collisions */
      G4ComponentAntiNuclNuclearXS* m_theAntiNucleonXS;
    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // ANTIBARYONPHYSICS_H
