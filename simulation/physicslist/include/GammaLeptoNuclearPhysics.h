/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GAMMALEPTONUCLEARPHYSICS_H
#define GAMMALEPTONUCLEARPHYSICS_H

#include "G4VPhysicsConstructor.hh"
#include "G4GammaParticipants.hh"
#include "G4QGSModel.hh"

class G4TheoFSGenerator;
class G4ExcitedStringDecay;
class G4QGSMFragmentation;
class G4GeneratorPrecompoundInterface;

namespace Belle2 {

  namespace Simulation {

    /**
     * Gamma-nuclear, electro-nuclear and muon-nuclear physics
     * constructor for Belle II physics list
     */
    class GammaLeptoNuclearPhysics: public G4VPhysicsConstructor {
    public:
      GammaLeptoNuclearPhysics();
      ~GammaLeptoNuclearPhysics();

      /** Build processes, models, cross sections used in physics list */
      virtual void ConstructProcess() override;

      /** Build all particle types used in physics list (empty in this class) */
      virtual void ConstructParticle() override;

    private:
      /** Final state generator for QCD string models */
      G4TheoFSGenerator* m_qgsp;

      /** Quark Gluon String model */
      G4QGSModel<G4GammaParticipants>* m_stringModel;

      /** Model to decay strings into hadrons */
      G4ExcitedStringDecay* m_stringDecay;

      /** Quark Gluon String fragmentation model */
      G4QGSMFragmentation* m_fragModel;

      /** Precompound model to deexcite post-collision nucleus */
      G4GeneratorPrecompoundInterface* m_preCompoundModel;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // GAMMALEPTONUCLEARPHYSICS_H
