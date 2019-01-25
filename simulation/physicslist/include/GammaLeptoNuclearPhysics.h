/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

      virtual void ConstructProcess() override;
      virtual void ConstructParticle() override;

    private:
      G4TheoFSGenerator* qgsp;
      G4QGSModel<G4GammaParticipants>* stringModel;
      G4ExcitedStringDecay* stringDecay;
      G4QGSMFragmentation* fragModel;
      G4GeneratorPrecompoundInterface* preCompoundModel;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // GAMMALEPTONUCLEARPHYSICS_H
