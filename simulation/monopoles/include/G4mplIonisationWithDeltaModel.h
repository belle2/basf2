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

#include <G4VEmModel.hh>
#include <G4VEmFluctuationModel.hh>
#include <vector>

class G4ParticleChangeForLoss;

namespace Belle2 {

  namespace Monopoles {

    class G4mplIonisationWithDeltaModel : public G4VEmModel, public G4VEmFluctuationModel {

    public:

      explicit G4mplIonisationWithDeltaModel(G4double mCharge,
                                             const G4String& nam = "mplIonisationWithDelta");

      virtual ~G4mplIonisationWithDeltaModel();

      virtual void Initialise(const G4ParticleDefinition*,
                              const G4DataVector&) override;

      virtual G4double ComputeDEDXPerVolume(const G4Material*,
                                            const G4ParticleDefinition*,
                                            G4double kineticEnergy,
                                            G4double cutEnergy) override;

      virtual G4double ComputeCrossSectionPerElectron(
        const G4ParticleDefinition*,
        G4double kineticEnergy,
        G4double cutEnergy,
        G4double maxEnergy);

      virtual G4double ComputeCrossSectionPerAtom(
        const G4ParticleDefinition*,
        G4double kineticEnergy,
        G4double Z, G4double A,
        G4double cutEnergy,
        G4double maxEnergy) override;

      virtual void SampleSecondaries(std::vector<G4DynamicParticle*>*,
                                     const G4MaterialCutsCouple*,
                                     const G4DynamicParticle*,
                                     G4double tmin,
                                     G4double maxEnergy) override;


      virtual G4double SampleFluctuations(const G4MaterialCutsCouple*,
                                          const G4DynamicParticle*,
                                          G4double tmax,
                                          G4double length,
                                          G4double meanLoss) override;

      virtual G4double Dispersion(const G4Material*,
                                  const G4DynamicParticle*,
                                  G4double tmax,
                                  G4double length) override;

      void SetParticle(const G4ParticleDefinition* p);

    protected:

      virtual G4double MaxSecondaryEnergy(const G4ParticleDefinition*,
                                          G4double kinEnergy) override;

    private:

      G4double ComputeDEDXAhlen(const G4Material* material, G4double bg2, G4double cut);

      // hide assignment operator
      G4mplIonisationWithDeltaModel&
      operator=(const  G4mplIonisationWithDeltaModel& right) = delete;
      G4mplIonisationWithDeltaModel(const  G4mplIonisationWithDeltaModel&) = delete;

      const G4ParticleDefinition* monopole;
      G4ParticleDefinition* theElectron;
      G4ParticleChangeForLoss*    fParticleChange;

      G4double mass;
      G4double magCharge;
      G4double twoln10;
      G4double betalow;
      G4double betalim;
      G4double beta2lim;
      G4double bg2lim;
      G4double chargeSquare;
      G4double dedxlim;
      G4double pi_hbarc2_over_mc2;

      static std::vector<G4double>* dedx0;
    };

  } //end Monopoles namespace

} //end Belle2 namespace
