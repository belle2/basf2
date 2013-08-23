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

    //! Calculate energy loss, fluctuation, and multiple-scattering angle for extrapolator
    class EnergyLossForExtrapolator {

    public:

      //! Constructor (with GEANT4 verbosity flag)
      EnergyLossForExtrapolator(G4int verb = 1);

      //! Destructor
      ~EnergyLossForExtrapolator();

      //! Get specific ionization energy loss for the given kinetic energy and particle
      G4double ComputeDEDX(G4double kinEnergy, const G4ParticleDefinition*);

      //! Get range for the given kinetic energy and particle
      G4double ComputeRange(G4double kinEnergy, const G4ParticleDefinition*);

      //! Get kinetic energy corresponding to the given range and particle
      G4double ComputeEnergy(G4double range, const G4ParticleDefinition*);

      //! Get kinetic energy after a step in given material by given particle
      G4double EnergyAfterStep(G4double kinEnergy, G4double step,
                               const G4Material*, const G4ParticleDefinition*);

      //! Get kinetic energy before a step in given material by given particle
      G4double EnergyBeforeStep(G4double kinEnergy, G4double step,
                                const G4Material*, const G4ParticleDefinition*);

      //! Get true step length for given particle and kinetic energy
      G4double TrueStepLength(G4double kinEnergy, G4double step,
                              const G4Material*, const G4ParticleDefinition* part);

      //! Get kinetic energy after a step in given material by given particle
      inline G4double EnergyAfterStep(G4double kinEnergy, G4double step,
                                      const G4Material*, const G4String& particleName);

      //! Get kinetic energy before a step in given material by given particle
      inline G4double EnergyBeforeStep(G4double kinEnergy, G4double step,
                                       const G4Material*, const G4String& particleName);

      //! Get average scattering angle after a step in given material by given particle
      inline G4double AverageScatteringAngle(G4double kinEnergy, G4double step,
                                             const G4Material*,
                                             const G4ParticleDefinition* part);

      //! Get average scattering angle after a step in given material by given particle
      inline G4double AverageScatteringAngle(G4double kinEnergy, G4double step,
                                             const G4Material*,
                                             const G4String& particleName);

      //! Get true step length for given particle and kinetic energy
      inline G4double ComputeTrueStep(const G4Material*, const G4ParticleDefinition* part,
                                      G4double kinEnergy, G4double stepLength);

      //! Get energy dispersion for a step in given material by given particle
      inline G4double EnergyDispersion(G4double kinEnergy, G4double step,
                                       const G4Material*, const G4ParticleDefinition*);

      //! Get energy dispersion for a step in given material by given particle
      inline G4double EnergyDispersion(G4double kinEnergy, G4double step,
                                       const G4Material*, const G4String& particleName);

      //! Change the GEANT4 verbosity for this class
      inline void SetVerbose(G4int val);

      //! Change the minimum particle kinetic energy for this class (default is 1 MeV)
      inline void SetMinKinEnergy(G4double);

      //! Change the maximum particle kinetic energy for this class (default is 10 TeV)
      inline void SetMaxKinEnergy(G4double);

      //! Change the maximum energy loss or gain for this class (default is infinity)
      inline void SetMaxEnergyTransfer(G4double);

    private:

      //! Initialize tables used to calculate energy loss, fluctuation, and scattering
      //! for electrons, positrons, muons, pions, kaons, and protons in all materials
      void Initialisation();

      //! Save current particle properties, kinetic energy and material in internal cached state
      G4bool SetupKinematics(const G4ParticleDefinition*, const G4Material*,
                             G4double kinEnergy);

      //! Create a new table to store one type of kinematics data for a particle
      G4PhysicsTable* PrepareTable();

      //! Get the particle definition of the named particle
      const G4ParticleDefinition* FindParticle(const G4String& name);

      //! Fill the table with the specific ionization energy loss of an electron
      void ComputeElectronDEDX(const G4ParticleDefinition* part, G4PhysicsTable* table);

      //! Fill the table with the specific ionization energy loss of a muon
      void ComputeMuonDEDX(const G4ParticleDefinition* part, G4PhysicsTable* table);

      //! Fill the table with the specific ionization energy loss of a hadron
      void ComputeHadronDEDX(const G4ParticleDefinition* part, G4PhysicsTable* table);

      //! Fill the table with the multiple-scattering cross section of a particle
      void ComputeTransportXS(const G4ParticleDefinition* part, G4PhysicsTable* table);

      //! Get the tabulated energy-loss, fluctuation, or scattering value for the given input
      inline G4double ComputeValue(G4double x, const G4PhysicsTable* table);

      //! Hide assignment operator
      EnergyLossForExtrapolator& operator=(const EnergyLossForExtrapolator& right);

      //! Hide copy-assignment operator
      EnergyLossForExtrapolator(const EnergyLossForExtrapolator&);

      //! Pointer to definition of the currently cached particle
      const G4ParticleDefinition* currentParticle;

      //! Pointer to definition of the electron
      const G4ParticleDefinition* electron;

      //! Pointer to definition of the positron
      const G4ParticleDefinition* positron;

      //! Pointer to definition of the positive muon
      const G4ParticleDefinition* muonPlus;

      //! Pointer to definition of the negative muon
      const G4ParticleDefinition* muonMinus;

      //! Pointer to definition of the positive pion
      const G4ParticleDefinition* pionPlus;

      //! Pointer to definition of the negative pion
      const G4ParticleDefinition* pionMinus;

      //! Pointer to definition of the positive kaon
      const G4ParticleDefinition* kaonPlus;

      //! Pointer to definition of the negative kaon
      const G4ParticleDefinition* kaonMinus;

      //! Pointer to definition of the proton
      const G4ParticleDefinition* proton;


      //! Pointer to the internal cache of default G4ProductionCuts
      G4ProductionCuts*        cuts;

      //! List of material-cuts pairings
      std::vector<const G4MaterialCutsCouple*> couples;

      //! Name of the internally cached particle
      G4String currentParticleName;

      //! Pointer to the electron's specific ionization energy loss vs KE table
      G4PhysicsTable*          dedxElectron;

      //! Pointer to the positron's specific ionization energy loss vs KE table
      G4PhysicsTable*          dedxPositron;

      //! Pointer to the muon's specific ionization energy loss vs KE table
      G4PhysicsTable*          dedxMuon;

      //! Pointer to the pion's specific ionization energy loss vs KE table
      G4PhysicsTable*          dedxPion;

      //! Pointer to the kaon's specific ionization energy loss vs KE table
      G4PhysicsTable*          dedxKaon;

      //! Pointer to the proton's specific ionization energy loss vs KE table
      G4PhysicsTable*          dedxProton;

      //! Pointer to the electron's range vs KE table
      G4PhysicsTable*          rangeElectron;

      //! Pointer to the positron's range vs KE table
      G4PhysicsTable*          rangePositron;

      //! Pointer to the muon's range vs KE table
      G4PhysicsTable*          rangeMuon;

      //! Pointer to the pion's range vs KE table
      G4PhysicsTable*          rangePion;

      //! Pointer to the kaon's range vs KE table
      G4PhysicsTable*          rangeKaon;

      //! Pointer to the proton's range vs KE table
      G4PhysicsTable*          rangeProton;

      //! Pointer to the electron's inverse-range vs KE table
      G4PhysicsTable*          invRangeElectron;

      //! Pointer to the positron's inverse-range vs KE table
      G4PhysicsTable*          invRangePositron;

      //! Pointer to the muon's inverse-range vs KE table
      G4PhysicsTable*          invRangeMuon;

      //! Pointer to the pion's inverse-range vs KE table
      G4PhysicsTable*          invRangePion;

      //! Pointer to the kaon's inverse-range vs KE table
      G4PhysicsTable*          invRangeKaon;

      //! Pointer to the proton's inverse-range vs KE table
      G4PhysicsTable*          invRangeProton;

      //! Pointer to the electron's multiple-scattering cross section vs KE table
      G4PhysicsTable*          mscElectron;

      //! Pointer to internally cached material
      const G4Material* currentMaterial;

      //! Cached material index
      G4int       index;

      //! Cached material electron density
      G4double    electronDensity;

      //! Cached material radiation length
      G4double    radLength;

      //! Cached particle mass
      G4double    mass;

      //! Cached charge-squared (in units of e)
      G4double    charge2;

      //! Cached particle kinetic energy
      G4double    kineticEnergy;

      //! Cached particle's gamma value
      G4double    gam;

      //! Cached particle's beta*gamma squared
      G4double    bg2;

      //! Cached particle's beta squared
      G4double    beta2;

      //! Cached particle's maximum kinetic-energy loss
      G4double    tmax;

      //! Step-length limit in units of range (0.01); not modifiable by user
      G4double    linLossLimit;

      //! User's minimum kinetic energy for particles (default 1 MeV)
      G4double    emin;

      //! User's maximum kinetic energy for particles (default 10 MeV)
      G4double    emax;

      //! User's upper limit on maximum kinetic-energy loss (default infinity)
      G4double    maxEnergyTransfer;

      //! Number of bins in each energy loss, fluctuation, and multiple-scattering table (70, fixed)
      G4int       nbins;

      //! Number of materials in current geometry
      G4int       nmat;

      //! GEANT4 verbosity for this class
      G4int       verbose;

      //! Flag to indicate that Initialisation() method has been called
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
