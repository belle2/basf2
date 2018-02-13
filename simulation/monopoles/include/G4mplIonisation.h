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

#include <G4VEnergyLossProcess.hh>
#include <globals.hh>
#include <G4VEmModel.hh>

class G4Material;
class G4VEmFluctuationModel;

namespace Belle2 {

  namespace Monopoles {

    class G4mplIonisation : public G4VEnergyLossProcess {

    public:

      explicit G4mplIonisation(G4double mCharge = 0.0,
                               const G4String& name = "mplIoni");

      virtual ~G4mplIonisation();

      virtual G4bool IsApplicable(const G4ParticleDefinition& p) override;

      // Print out of the class parameters
      virtual void PrintInfo() override;

    protected:

      virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                               const G4ParticleDefinition*) override;

    private:

      // hide assignment operator
      G4mplIonisation& operator=(const G4mplIonisation& right) = delete;
      G4mplIonisation(const G4mplIonisation&) = delete;

      G4double    magneticCharge;
      G4bool      isInitialised;

    };

  } //end Monopoles namespace

} //end Belle2 namespace
