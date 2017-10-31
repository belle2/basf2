//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// modified from exoticphysics/monopole/*

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef G4MPLIONISATIONWITHDELTAMODEL_H
#define G4MPLIONISATIONWITHDELTAMODEL_H

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
      G4int    nmpl;
      G4double nmpl_in_eplus;
      G4double pi_hbarc2_over_mc2;

      static std::vector<G4double>* dedx0;
    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
