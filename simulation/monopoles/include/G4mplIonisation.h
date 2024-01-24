/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#pragma once

#include <G4VEnergyLossProcess.hh>
#include <globals.hh>

class G4Material;

namespace Belle2 {

  namespace Monopoles {

    /**
     * Monopole ionisation class.
     *
     * A wrapper, rather than a concrete class. Different ionisation models
     * can be plugged in.
     */
    class G4mplIonisation : public G4VEnergyLossProcess {

    public:

      /**
       * Constructor.
       * @param mCharge Monopole magnetic charge
       * @param name Name of the ionisation process in GEANT4
       */
      explicit G4mplIonisation(G4double mCharge = 0.0,
                               const G4String& name = "mplIoni");

      /**
       * Destructor.
       */
      virtual ~G4mplIonisation();

      /**
       * Dummy to check if the proccess is is applicable to a certain particle.
       * @param p Particle definition reference
       * @return true
       */
      virtual G4bool IsApplicable(const G4ParticleDefinition& p) override;

      /**
       * Threshold for zero value
       */
      virtual G4double MinPrimaryEnergy(const G4ParticleDefinition* p,
                                        const G4Material*, G4double cut) final;

      /**
       * Assignment operator should be hidden.
       * @param right Assign reference
       */
      G4mplIonisation& operator=(const G4mplIonisation& right) = delete;
      /**
       * Copy constructor should be hidden.
       * @param copy Copy reference
       */
      G4mplIonisation(const G4mplIonisation& copy) = delete;

    protected:

      /**G4VEnergyLossProcess::InitialiseEnergyLossProcess() implementation.
       * Initialise ionisation proccess, if not it has not been initialised yet.
       *
       * Concrete ionisation model is called inside this function.
       * @param p Particle definition pointer, the monopole in our case
       */
      virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition* p,
                                               const G4ParticleDefinition*) override;

    private:

      G4double    magneticCharge; /**< Monopole charge, in e+ units*/
      G4bool      isInitialised; /**< Is the process initialised*/

    };

  } //end Monopoles namespace

} //end Belle2 namespace
