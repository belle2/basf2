/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BELLE2PHYSICSLIST_H
#define BELLE2PHYSICSLIST_H

#include "G4VModularPhysicsList.hh"

namespace Belle2 {

  namespace Simulation {

    /**
     * Custom Geant4 physics list for Belle II with options to
     * add optical physics, standard EM physics and high precision
     * neutrons
     */
    class Belle2PhysicsList: public G4VModularPhysicsList {

    public:
      /** Constructor */
      explicit Belle2PhysicsList(const G4String& physicsListName);
      /** destructor */
      ~Belle2PhysicsList();

      /** Build all particle types used in physics list */
      virtual void ConstructParticle();

      /** Set the secondary particle production thresholds */
      virtual void SetCuts();

      /** Run/event verbosity level */
      void SetVerbosity(G4int verb);

      /** Use parameter to set cut value */
      void SetProductionCutValue(G4double);

      /** Use standard EM physics instead of EM option1 */
      void UseStandardEMPhysics(G4bool);

      /** Add optical photon physics */
      void UseOpticalPhysics(G4bool);

      /** Use high precision neutron models below 20 MeV */
      void UseHighPrecisionNeutrons(G4bool);

    private:
      /** Secondary production threshold */
      G4double m_globalCutValue;
    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // BELLE2PHYSICSLIST_H
