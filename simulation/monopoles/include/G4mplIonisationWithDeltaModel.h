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

    /**
     * Concrete monopole ionisation model.
     *
     * Monopole magnetic ionisation dedx is similar to
     * electric one with the exception of beta*beta factor.
     * This model is also capable of producing secondary delta electrons.
     */
    class G4mplIonisationWithDeltaModel : public G4VEmModel, public G4VEmFluctuationModel {

    public:

      /**
       * Constructor.
       *
       * @param mCharge Magnetic charge of the monopole, in e+ units
       * @param nam Name of the ionisation process in GEANT4
       */
      explicit G4mplIonisationWithDeltaModel(G4double mCharge,
                                             const G4String& nam = "mplIonisationWithDelta");

      /**
       * Destructor.
       */
      virtual ~G4mplIonisationWithDeltaModel();

      /**
       * G4VEmModel::Initialise implementation.
       *
       * @param p Monopole definition pointer.
       */
      virtual void Initialise(const G4ParticleDefinition* p,
                              const G4DataVector&) override;

      /**
       * G4VEmModel::ComputeDEDXPerVolume implementation.
       *
       * Combines dedx estimation from extrapolated Ahlen formula
       * and formula for low beta.
       *
       * @param material Pointer to the material of current volume
       * @param p Pointer to monopole definition
       * @param kineticEnergy Monopole kinetic energy value
       * @param maxEnergy Maximum energy of secondary electron
       * @return dedx magnitude
       */
      virtual G4double ComputeDEDXPerVolume(const G4Material* material,
                                            const G4ParticleDefinition* p,
                                            G4double kineticEnergy,
                                            G4double maxEnergy) override;

      /**
       * Compute cross section per electron for delta electrons emission.
       *
       * @param p Pointer to monopole definition
       * @param kinEnergy Monopole kinetic energy value
       * @param cutEnergy Cut energy for ionisation process
       * @param maxEnergy Maximum energy of secondary electron
       * @return Cross section
       */
      virtual G4double ComputeCrossSectionPerElectron(
        const G4ParticleDefinition* p,
        G4double kineticEnergy,
        G4double cutEnergy,
        G4double maxEnergy);

      /**
       * Compute cross section per atom for delta electrons emission.
       *
       * @param p Pointer to monopole definition
       * @param kinEnergy Monopole kinetic energy value
       * @param Z Atomic charge
       * @param A This argument of base function is ignored
       * @param cutEnergy Cut energy for ionisation process
       * @param maxEnergy Maximum energy of secondary electron
       * @return Cross section
       */
      virtual G4double ComputeCrossSectionPerAtom(
        const G4ParticleDefinition* p,
        G4double kineticEnergy,
        G4double Z, G4double A,
        G4double cutEnergy,
        G4double maxEnergy) override;

      /**
       * Create the sample of secondary delta electrons.
       *
       * @param vdp Pointer to the storage of sampled delta electrons
       * @param materialCutsCouple This argument of base function is ignored
       * @param dp Secnodary particle pointer
       * @param minKinEnergy Minimal energy of secondary electrons
       * @param maxEnergy Maximum energy of secondary electrons
       */
      virtual void SampleSecondaries(std::vector<G4DynamicParticle*>* vdp,
                                     const G4MaterialCutsCouple* materialCutsCouple,
                                     const G4DynamicParticle* dp,
                                     G4double minKinEnergy,
                                     G4double maxEnergy) override;


      /**
       * Create fluctuations in the energies lost to a secondary delta electron.
       *
       * @param couple Current volume coulple to get its material
       * @param dp Secnodary particle pointer
       * @param tmax Tmax in dispersion formula?
       * @param length Length in dispersion formula?
       * @param meanLoss Mean of energy loss
       * @return Fluctuated loss
       */
      virtual G4double SampleFluctuations(const G4MaterialCutsCouple* couple,
                                          const G4DynamicParticle* dp,
                                          G4double tmax,
                                          G4double length,
                                          G4double meanLoss) override;

      /**
       * Calculate dispersion.
       *
       * @param material Pointer to current material definition
       * @param dp Secnodary particle pointer
       * @param tmax Tmax in dispersion formula?
       * @param length Length in dispersion formula?
       * @return Dispersion
       */
      virtual G4double Dispersion(const G4Material* material,
                                  const G4DynamicParticle*,
                                  G4double tmax,
                                  G4double length) override;

      void SetParticle(const G4ParticleDefinition* p); /**< Read definition of the monopole*/

    protected:

      /**
       * Calculate maximum energy available for secondary particle emission.
       *
       * @param p This argument of base function is ignored
       * @param kinEnergy Kinetic energy of the ionising particle
       * @return Maximum energy available for secondary particle emission
       */
      virtual G4double MaxSecondaryEnergy(const G4ParticleDefinition* p,
                                          G4double kinEnergy) override;

    private:

      /**
       * Caculate dedx based on extrapolated Ahlen formula.
       *
       * References
       * [1] Steven P. Ahlen: Energy loss of relativistic heavy ionizing particles,
       *     S.P. Ahlen, Rev. Mod. Phys 52(1980), p121
       * [2] K.A. Milton arXiv:hep-ex/0602040
       * [3] S.P. Ahlen and K. Kinoshita, Phys. Rev. D26 (1982) 2347
       *
       * @param material Pointer to the material of current volume
       * @param bg2 (beta*gamma)^2
       * @param cut Cut energy for ionisation process
       * @return dedx magnitude
       */
      G4double ComputeDEDXAhlen(const G4Material* material, G4double bg2, G4double cut);

      /**
       * Assignment operator should be hidden.
       *
       * @param right Assign reference
       */
      G4mplIonisationWithDeltaModel&
      operator=(const  G4mplIonisationWithDeltaModel& right) = delete;
      /**
       * Copy constructor should be hidden.
       *
       * @param copy Copy reference
       */
      G4mplIonisationWithDeltaModel(const  G4mplIonisationWithDeltaModel& copy) = delete;

      const G4ParticleDefinition* monopole; /**< Monopole definition*/
      G4ParticleDefinition* theElectron; /**< Electron definition*/
      G4ParticleChangeForLoss*    fParticleChange; /**< Pointer to ionising particle*/

      G4double mass; /**< Mass of the monopole */
      G4double magCharge; /**< Monopole magnetic charge*/
      //FIXME following is lazy programming
      G4double twoln10; /**< log(100.0)*/
      G4double betalow; /**< Beta threshold for low asymptotic*/
      G4double betalim; /**< Beta threshold for high energy (only Ahlen formula)*/
      G4double beta2lim; /**< Square of betalim*/
      G4double bg2lim; /**< (beta*gamma)^2 for betalim*/
      G4double chargeSquare; /**< Square of magnetic charge*/
      G4double dedxlim; /**< dedx limit in asymptotic formula, not used*/
      G4double pi_hbarc2_over_mc2; /**< Mass with constants combination*/

      static std::vector<G4double>* dedx0; /**<Base dedx for each couple in current material*/
    };

  } //end Monopoles namespace

} //end Belle2 namespace
