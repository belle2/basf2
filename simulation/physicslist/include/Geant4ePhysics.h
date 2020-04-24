/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dennis Wright                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEANT4EPHYSICS_H_
#define GEANT4EPHYSICS_H_

#include <globals.hh>
#include <G4VPhysicsConstructor.hh>

namespace Belle2 {

  namespace Simulation {

    class ExtStepLengthLimitProcess;
    class ExtMagFieldLimitProcess;
    class ExtEnergyLoss;
    class ExtMessenger;

    /**
     * Define geant4e-specific physics
     */
    class Geant4ePhysics: public G4VPhysicsConstructor {

    public:

      /** The Geant4ePhysics constructor */
      Geant4ePhysics();

      /** The Geant4ePhysics destructor */
      virtual ~Geant4ePhysics();

      /** ConstructParticle() defines the geant4e-specific particles */
      virtual void ConstructParticle() override;

      /** ConstructProcess() defines the geant4e-specific processes */
      virtual void ConstructProcess() override;

    private:

      /** Process that limits the geant4e step length */
      ExtStepLengthLimitProcess* m_StepLengthLimitProcess;

      /** Process that limits the geant4e step length in magnetic field */
      ExtMagFieldLimitProcess* m_MagFieldLimitProcess;

      /** Process that limits the geant4e step length due to energy loss */
      ExtEnergyLoss* m_ELossProcess;

      /** Pointer to the ExtMessenger that is used to control geant4e */
      ExtMessenger* m_Messenger;

    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* GEANT4EPHYSICS_H_ */
