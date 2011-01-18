/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4PHYSICSLIST_H
#define B4PHYSICSLIST_H

#include "G4VModularPhysicsList.hh"

namespace Belle2 {

  //! The class for Belle2 Physics List

  class B4PhysicsList: public G4VModularPhysicsList {
  public:

    B4PhysicsList(G4String, G4bool);
    virtual ~B4PhysicsList();

    void SetCuts();

  private:

  };
} // end namespace Belle2
#endif /* B4PHYSICSLIST_H */
