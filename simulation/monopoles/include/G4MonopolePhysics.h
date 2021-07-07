/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#pragma once

#include <G4VPhysicsConstructor.hh>
#include <globals.hh>
#include <simulation/monopoles/MonopoleConstants.h>

namespace Belle2 {

  namespace Monopoles {

    class G4Monopole;

    /**
     * Monopole physics class to register on the physics list.
     *
     * Entry point from FullSim to the monopole package. Consists of monopole
     * definition, transportation and ionisation processes.
     */
    class G4MonopolePhysics : public G4VPhysicsConstructor {
    public:

      /**
       * Constructor.
       * @param magneticCharge Magnetic charge of the monopole, in e+ units.
       */
      explicit G4MonopolePhysics(double magneticCharge = 1.0);

      /**
       * Destructor.
       */
      ~G4MonopolePhysics();

      /**
       * Adds monopole and anti-monopole to GEANT4 with a pdg of +/-99666
       * and parameters taken from current class.
       */
      virtual void ConstructParticle();

      /**
      * This method will be invoked in the Construct() method.
      * each physics process will be instantiated and
      * registered to the process manager of each particle type.
      */
      virtual void ConstructProcess();

    private:

      /**
       * Assignment operator should be hidden.
       * @param right Assign reference.
       */
      G4MonopolePhysics& operator=(const G4MonopolePhysics& right);
      /**
       * Copy constructor should be hidden.
       * @param copy Copy reference.
       */
      G4MonopolePhysics(const G4MonopolePhysics& copy);

      G4double    fMagCharge; /**< Magnetic charge of the monopole, in e+ units*/
      G4double    fElCharge; /**< Electric charge in case of dyon*/
      G4double    fMonopoleMass; /**< Mass of the monopole*/

      G4Monopole* fMpl; /**< Pointer to the monopole definition in GEANT4*/
      G4Monopole* fApl; /**< Pointer to the anti-monopole definition in GEANT4*/
      //Both monopole and anti-monopole have own process managers and have to be treated separately

    };

  } //end Monopoles namespace

} //end Belle2 namespace
