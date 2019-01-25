/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef IONPHYSICS_H
#define IONPHYSICS_H

#include "G4VPhysicsConstructor.hh"

class G4TheoFSGenerator;
class G4FTFModel;
class G4ExcitedStringDecay;
class G4LundStringFragmentation;
class G4GeneratorPrecompoundInterface;
class G4VComponentCrossSection;
class G4ComponentGGNuclNuclXsc;

namespace Belle2 {

  namespace Simulation {

    /**
     * Ion hadronic physics constructor for Belle II physics list
     */
    class IonPhysics: public G4VPhysicsConstructor {
    public:
      IonPhysics();
      ~IonPhysics();

      virtual void ConstructParticle() override;
      virtual void ConstructProcess() override;

    private:
      G4TheoFSGenerator* ftfp;
      G4FTFModel* stringModel;
      G4ExcitedStringDecay* stringDecay;
      G4LundStringFragmentation* fragModel;
      G4GeneratorPrecompoundInterface* preCompoundModel;

      G4VComponentCrossSection* theGGNuclNuclXS;
      G4ComponentGGNuclNuclXsc* ionGGXS;
    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // IONPHYSICS_H
