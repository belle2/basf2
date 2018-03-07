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

#include <G4ParticleDefinition.hh>
#include <globals.hh>

#include <CLHEP/Units/SystemOfUnits.h>

namespace Belle2 {

  namespace Monopoles {
    /**
    * A class to hold monopole description as a particle.
    *
    * Creates GEANT4 particle via G4ParticleDefinition and adds extra member for magnetic charge.
    * Particle properties like spin, parities etc. are fixed by default to some values,
    * even though they might be inconsistent with each other. Nevertheless, since monopoles do not decay
    * and are produced only in the generator, that should not matter.
    */
    class G4Monopole : public G4ParticleDefinition {
    private:

      /**
       * Destructor.
       */
      virtual ~G4Monopole();

    public:

      /**
       * Constructor.
       *
       * @param name Name of the monople particle in GEANT4
       * @param mass Mass of the monopole
       * @param mCharge Magnetic charge of the monopole, in e+ units
       * @param eCharge Electric charge in case of a dyon
       * @param encoding PDG code of the monopole. Anything starting with "99" will work.
       */
      G4Monopole(const G4String& name,
                 G4double  mass,
                 G4double  mCharge,
                 G4double  eCharge,
                 G4int     encoding);

      /**
       * Returns magnetic charge of the monopole
       * @return magnetic charge of the monopole
       */
      G4double MagneticCharge() const;

    private:

      G4double magCharge; /**< Magnetic charge of the monopole, in e+ units */
    };

  } //end Monopoles namespace

} //end Belle2 namespace
