/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PHYSICSLIST_H_
#define PHYSICSLIST_H_

#include <globals.hh>
#include <G4VModularPhysicsList.hh>

#include <string>

namespace Belle2 {

  namespace Simulation {

    /**
     * The basf2 physics list.
     * Uses the Geant4 standard lists, specified by a string and adds optical processes
     * if requested.
     */
    class PhysicsList: public G4VModularPhysicsList {

    public:

      /**
       * The PhysicsList constructor.
       * Loads the physics list specified by its name.
       * @param physicsListName The name of the physics list which should be loaded.
       */
      explicit PhysicsList(const std::string& physicsListName);

      /** The PhysicsList destructor. */
      virtual ~PhysicsList();

      /** Sets the Cuts on the physics list. */
      void SetCuts();

      /**
       * Sets the production cut value.
       * If a primary particle has no longer enough energy to produce secondaries
       * which travel at least the specified productionCut distance, two things happen:
       * 1) Discrete energy loss ceases (no more secondaries will be produced)
       * 2) The primary particle is tracked down to zero energy using continuous energy loss
       * @param productionCut The production cut value in [cm].
       */
      void setProductionCutValue(double productionCut);

      /** Registers the optical physics list. */
      void registerOpticalPhysicsList();


    private:

    };

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* PHYSICSLIST_H_ */
