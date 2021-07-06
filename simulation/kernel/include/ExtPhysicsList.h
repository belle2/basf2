/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef EXTPHYSICSLIST_H
#define EXTPHYSICSLIST_H

#include <globals.hh>
#include <G4VModularPhysicsList.hh>

namespace Belle2 {

  namespace Simulation {

    /**
     * Standalone physics list for the geant4e extrapolator.
     * This is used only if the simulation module is not present; otherwise,
     * the simulation's physics list (which contains this) is used.
     */
    class ExtPhysicsList: public G4VModularPhysicsList {

    public:

      //! Constructor
      ExtPhysicsList();

      //! Destructor
      virtual ~ExtPhysicsList();

      //! Required method: sets cutoff values for propagation
      void SetCuts();

    private:

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // EXTPHYSICSLIST_H
