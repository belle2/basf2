/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#pragma once

#include <G4VPhysicsConstructor.hh>
#include <globals.hh>

namespace Belle2 {

  namespace Monopoles {

    class G4Monopole;

    class G4MonopolePhysics : public G4VPhysicsConstructor {
    public:

      G4MonopolePhysics(double magneticCharge = 1.0);

      ~G4MonopolePhysics();

      // This method is dummy for physics
      virtual void ConstructParticle();

      // This method will be invoked in the Construct() method.
      // each physics process will be instantiated and
      // registered to the process manager of each particle type
      virtual void ConstructProcess();

    private:

      // hide assignment operator
      G4MonopolePhysics& operator=(const G4MonopolePhysics& right);
      G4MonopolePhysics(const G4MonopolePhysics&);

      G4double    fMagCharge;
      G4double    fElCharge;
      G4double    fMonopoleMass;

      G4Monopole* fMpl; //Monopole
      G4Monopole* fApl; //Anti_Monopole
      //Anti-monopole and its processes has to be treated separetely
      //TODO fix physics process registration so that anti-monopole can be treated automatically
      //as a an anti-particle of the monopole

    };

  } //end Monopoles namespace

} //end Belle2 namespace
