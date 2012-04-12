/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTPHYSICSLIST_H
#define EXTPHYSICSLIST_H

#include <globals.hh>
#include <G4VModularPhysicsList.hh>

namespace Belle2 {

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

} // end of namespace Belle2

#endif // EXTPHYSICSLIST_H
