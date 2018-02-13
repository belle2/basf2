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

    class G4Monopole : public G4ParticleDefinition {
    private:

      virtual ~G4Monopole();

    public:

      G4Monopole(const G4String& name,
                 G4double  mass,
                 G4double  mCharge,
                 G4double  eCharge,
                 G4int     encoding);

      G4double MagneticCharge() const;

    private:

      G4double magCharge;
    };

  } //end Monopoles namespace

} //end Belle2 namespace
