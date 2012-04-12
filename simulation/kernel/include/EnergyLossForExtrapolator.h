/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 * Derived from: EnergyLossForExtrapolator.hh                             *
 *               (use geant4e-specific particles; include pions & kaons)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ENERGYLOSSFOREXTRAPOLATOR_H_
#define ENERGYLOSSFOREXTRAPOLATOR_H_

#include <globals.hh>
#include <G4PhysicsTable.hh>
#include <vector>

class G4ParticleDefinition;
class G4Material;
class G4MaterialCutsCouple;
class G4ProductionCuts;

namespace Belle2 {

  namespace Simulation {

    //! Calculate energy loss, fluctuation,and multiple-scattering angle for extrapolator
    class EnergyLossForExtrapolator {
    public:
      EnergyLossForExtrapolator(G4int verb = 1);

      ~EnergyLossForExtrapolator();

      G4double ComputeDEDX(G4double kinEnergy, const G4ParticleDefinition*);

      G4double ComputeRange(G4double kinEnergy, const G4ParticleDefinition*);

      G4double ComputeEnergy(G4double range, const G4ParticleDefinition*);

      G4double EnergyAfterStep(G4double kinEnergy, G4double step,
                               const G4Material*, const G4ParticleDefinition*);

      G4double EnergyBeforeStep(G4double kinEnergy, G4double step,
                                const G4Material*, const G4ParticleDefinition*);

      G4double TrueStepLength(G4double kinEnergy, G4double step,
                              const G4Material*, const G4ParticleDefinition* part);

      inline G4double EnergyAfterStep(G4double kinEnergy, G4double step,
                                      const G4Material*, const G4String& particleName);

      inline G4double EnergyBeforeStep(G4double kinEnergy, G4double step,
                                       const G4Material*, const G4String& particleName);

      inline G4double AverageScatteringAngle(G4double kinEnergy, G4double step,
                                             const G4Material*,
                                             const G4ParticleDefinition* part);

      inline G4double AverageScatteringAngle(G4double kinEnergy, G4double step,
                                             const G4Material*,
                                             const G4String& particleName);

      inline G4double ComputeTrueStep(const G4Material*, const G4ParticleDefinition* part,
                                      G4double kinEnergy, G4double stepLength);

      inline G4double EnergyDispersion(G4double kinEnergy, G4double step,
                                       const G4Material*, const G4ParticleDefinition*);

      inline G4double EnergyDispersion(G4double kinEnergy, G4double step,
                                       const G4Material*, const G4String& particleName);

      inline void SetVerbose(G4int val);

      inline void SetMinKinEnergy(G4double);

      inline void SetMaxKinEnergy(G4double);

      inline void SetMaxEnergyTransfer(G4double);

    private:

      void Initialisation();

      G4bool SetupKinematics(const G4ParticleDefinition*, const G4Material*,
                             G4double kinEnergy);

      G4PhysicsTable* PrepareTable();

      const G4ParticleDefinition* FindParticle(const G4String& name);

      void ComputeElectronDEDX(const G4ParticleDefinition* part, G4PhysicsTable* table);

      void ComputeMuonDEDX(const G4ParticleDefinition* part, G4PhysicsTable* table);

      void ComputeHadronDEDX(const G4ParticleDefinition* part, G4PhysicsTable* table);

      void ComputeTransportXS(const G4ParticleDefinition* part, G4PhysicsTable* table);

      inline G4double ComputeValue(G4double x, const G4PhysicsTable* table);

      // hide assignment operator
      EnergyLossForExtrapolator& operator=(const EnergyLossForExtrapolator& right);
      EnergyLossForExtrapolator(const EnergyLossForExtrapolator&);

      const G4ParticleDefinition* currentParticle;
      const G4ParticleDefinition* electron;
      const G4ParticleDefinition* positron;
      const G4ParticleDefinition* muonPlus;
      const G4ParticleDefinition* muonMinus;
      const G4ParticleDefinition* pionPlus;
      const G4ParticleDefinition* pionMinus;
      const G4ParticleDefinition* kaonPlus;
      const G4ParticleDefinition* kaonMinus;
      const G4ParticleDefinition* proton;

      G4ProductionCuts*        cuts;
      std::vector<const G4MaterialCutsCouple*> couples;

      G4String currentParticleName;

      G4PhysicsTable*          dedxElectron;
      G4PhysicsTable*          dedxPositron;
      G4PhysicsTable*          dedxMuon;
      G4PhysicsTable*          dedxPion;
      G4PhysicsTable*          dedxKaon;
      G4PhysicsTable*          dedxProton;
      G4PhysicsTable*          rangeElectron;
      G4PhysicsTable*          rangePositron;
      G4PhysicsTable*          rangeMuon;
      G4PhysicsTable*          rangePion;
      G4PhysicsTable*          rangeKaon;
      G4PhysicsTable*          rangeProton;
      G4PhysicsTable*          invRangeElectron;
      G4PhysicsTable*          invRangePositron;
      G4PhysicsTable*          invRangeMuon;
      G4PhysicsTable*          invRangePion;
      G4PhysicsTable*          invRangeKaon;
      G4PhysicsTable*          invRangeProton;
      G4PhysicsTable*          mscElectron;

      const G4Material* currentMaterial;
      G4int       index;
      G4double    electronDensity;
      G4double    radLength;
      G4double    mass;
      G4double    charge2;
      G4double    kineticEnergy;
      G4double    gam;
      G4double    bg2;
      G4double    beta2;
      G4double    tmax;

      G4double    linLossLimit;
      G4double    emin;
      G4double    emax;
      G4double    maxEnergyTransfer;

      G4int       nbins;
      G4int       nmat;
      G4int       verbose;
      G4bool      isInitialised;
    };

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double EnergyLossForExtrapolator::EnergyAfterStep(G4double kinEnergy,
                                                               G4double step,
                                                               const G4Material* mat,
                                                               const G4String& name)
    {
      return EnergyAfterStep(kinEnergy, step, mat, FindParticle(name));
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double EnergyLossForExtrapolator::EnergyBeforeStep(G4double kinEnergy,
                                                                G4double step,
                                                                const G4Material* mat,
                                                                const G4String& name)
    {
      return EnergyBeforeStep(kinEnergy, step, mat, FindParticle(name));
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double EnergyLossForExtrapolator::AverageScatteringAngle(G4double kinEnergy,
        G4double step,
        const G4Material* mat,
        const G4String& name)
    {
      return AverageScatteringAngle(kinEnergy, step, mat, FindParticle(name));
    }
    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double
    EnergyLossForExtrapolator::EnergyDispersion(G4double kinEnergy,
                                                G4double step,
                                                const G4Material* mat,
                                                const G4String& name)
    {
      return EnergyDispersion(kinEnergy, step, mat, FindParticle(name));
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double
    EnergyLossForExtrapolator::AverageScatteringAngle(G4double kinEnergy,
                                                      G4double stepLength,
                                                      const G4Material* mat,
                                                      const G4ParticleDefinition* part)
    {
      G4double theta = 0.0;
      if (SetupKinematics(part, mat, kinEnergy)) {
        G4double t = stepLength / radLength;
        G4double y = std::max(0.001, t);
        theta = 19.23 * MeV * std::sqrt(charge2 * t) * (1.0 + 0.038 * std::log(y)) / (beta2 * gam * mass);
      }
      return theta;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double
    EnergyLossForExtrapolator::ComputeTrueStep(const G4Material* mat,
                                               const G4ParticleDefinition* part,
                                               G4double kinEnergy,
                                               G4double stepLength)
    {
      G4double theta = AverageScatteringAngle(kinEnergy, stepLength, mat, part);
      return stepLength * std::sqrt(1.0 + 0.625 * theta * theta);
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline
    G4double EnergyLossForExtrapolator::EnergyDispersion(G4double kinEnergy,
                                                         G4double stepLength,
                                                         const G4Material* mat,
                                                         const G4ParticleDefinition* part)
    {
      G4double sig2 = 0.0;
      if (SetupKinematics(part, mat, kinEnergy)) {
        G4double step = ComputeTrueStep(mat, part, kinEnergy, stepLength);
        sig2 = (1.0 / beta2 - 0.5) * twopi_mc2_rcl2 * tmax * step * electronDensity * charge2;
      }
      return sig2;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double EnergyLossForExtrapolator::ComputeValue(G4double x,
                                                            const G4PhysicsTable* table)
    {
      G4double res = 0.0;
      G4bool b;
      if (table) res = ((*table)[index])->GetValue(x, b);
      return res;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetVerbose(G4int val)
    {
      verbose = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetMinKinEnergy(G4double val)
    {
      emin = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetMaxKinEnergy(G4double val)
    {
      emax = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetMaxEnergyTransfer(G4double val)
    {
      maxEnergyTransfer = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* ENERGYLOSSFOREXTRAPOLATOR_H_ */
