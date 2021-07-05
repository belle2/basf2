/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ENERGYLOSSFOREXTRAPOLATOR_H_
#define ENERGYLOSSFOREXTRAPOLATOR_H_

#include <CLHEP/Units/SystemOfUnits.h>

#include <globals.hh>
#include <G4PhysicsTable.hh>
#include <G4DataVector.hh>
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

      //! Constructor (*WITHOUT* GEANT4 verbosity flag)
      EnergyLossForExtrapolator();

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
      //- inline G4double EnergyAfterStep(G4double kinEnergy, G4double step,
      //-                                const G4Material*, const G4String& particleName);

      //! Get kinetic energy before a step in given material by given particle
      //- inline G4double EnergyBeforeStep(G4double kinEnergy, G4double step,
      //-                                  const G4Material*, const G4String& particleName);

      //! Get average scattering angle after a step in given material by given particle
      inline G4double AverageScatteringAngle(G4double kinEnergy, G4double step,
                                             const G4Material*,
                                             const G4ParticleDefinition* part);

      //! Get average scattering angle after a step in given material by given particle
      //- inline G4double AverageScatteringAngle(G4double kinEnergy, G4double step,
      //-                                        const G4Material*,
      //-                                        const G4String& particleName);

      //! Get true step length for given particle and kinetic energy
      inline G4double ComputeTrueStep(const G4Material*, const G4ParticleDefinition* part,
                                      G4double kinEnergy, G4double stepLength);

      //! Get energy dispersion for a step in given material by given particle
      //- inline G4double EnergyDispersion(G4double kinEnergy, G4double step,
      //-                                  const G4Material*, const G4ParticleDefinition*);

      //! Get energy dispersion for a step in given material by given particle
      //- inline G4double EnergyDispersion(G4double kinEnergy, G4double step,
      //-                                  const G4Material*, const G4String& particleName);

      //! Change the GEANT4 verbosity for this class
      //- inline void SetVerbose(G4int val);

      //! Change the minimum particle kinetic energy for this class (default is 1 MeV)
      inline void SetMinKinEnergy(G4double);

      //! Change the maximum particle kinetic energy for this class (default is 10 TeV)
      inline void SetMaxKinEnergy(G4double);

      //! Change the maximum energy loss or gain for this class (default is infinity)
      inline void SetMaxEnergyTransfer(G4double);

    private:

      //! Initialize tables used to calculate energy loss, fluctuation, and scattering
      //! for electrons, positrons, muons, pions, kaons, protons  and deuterons in all materials
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
      const G4ParticleDefinition* m_Particle;

      //! Pointer to definition of the electron
      const G4ParticleDefinition* m_Electron;

      //! Pointer to definition of the positron
      const G4ParticleDefinition* m_Positron;

      //! Pointer to definition of the positive muon
      const G4ParticleDefinition* m_MuonPlus;

      //! Pointer to definition of the negative muon
      const G4ParticleDefinition* m_MuonMinus;

      //! Pointer to definition of the positive pion
      const G4ParticleDefinition* m_PionPlus;

      //! Pointer to definition of the negative pion
      const G4ParticleDefinition* m_PionMinus;

      //! Pointer to definition of the positive kaon
      const G4ParticleDefinition* m_KaonPlus;

      //! Pointer to definition of the negative kaon
      const G4ParticleDefinition* m_KaonMinus;

      //! Pointer to definition of the proton
      const G4ParticleDefinition* m_Proton;

      //! Pointer to definition of the antiproton
      const G4ParticleDefinition* m_AntiProton;

      //! Pointer to definition of the deuteron
      const G4ParticleDefinition* m_Deuteron;

      //! Pointer to definition of the antideuteron
      const G4ParticleDefinition* m_AntiDeuteron;

      //! Vector of particle cuts
      G4DataVector m_Cuts;

      //! Pointer to the internal cache of default G4ProductionCuts
      G4ProductionCuts* m_ProductionCuts;

      //! List of material-cuts pairings
      std::vector<const G4MaterialCutsCouple*> m_Couples;

      //! Pointer to the electron's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxElectron;

      //! Pointer to the positron's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxPositron;

      //! Pointer to the muon's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxMuon;

      //! Pointer to the pion's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxPion;

      //! Pointer to the kaon's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxKaon;

      //! Pointer to the proton's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxProton;

      //! Pointer to the deuteron's specific ionization energy loss vs KE table
      G4PhysicsTable*          m_DedxDeuteron;

      //! Pointer to the electron's range vs KE table
      G4PhysicsTable*          m_RangeElectron;

      //! Pointer to the positron's range vs KE table
      G4PhysicsTable*          m_RangePositron;

      //! Pointer to the muon's range vs KE table
      G4PhysicsTable*          m_RangeMuon;

      //! Pointer to the pion's range vs KE table
      G4PhysicsTable*          m_RangePion;

      //! Pointer to the kaon's range vs KE table
      G4PhysicsTable*          m_RangeKaon;

      //! Pointer to the proton's range vs KE table
      G4PhysicsTable*          m_RangeProton;

      //! Pointer to the deuteron's range vs KE table
      G4PhysicsTable*          m_RangeDeuteron;

      //! Pointer to the electron's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangeElectron;

      //! Pointer to the positron's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangePositron;

      //! Pointer to the muon's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangeMuon;

      //! Pointer to the pion's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangePion;

      //! Pointer to the kaon's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangeKaon;

      //! Pointer to the proton's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangeProton;

      //! Pointer to the deuteron's inverse-range vs KE table
      G4PhysicsTable*          m_InvRangeDeuteron;

      //! Pointer to the electron's multiple-scattering cross section vs KE table
      G4PhysicsTable*          m_MscatElectron;

      //! Pointer to internally cached material
      const G4Material* m_Material;

      //! Cached material index
      G4int       m_MaterialIndex;

      //! Cached material electron density
      G4double    m_ElectronDensity;

      //! Cached material radiation length
      G4double    m_RadLength;

      //! Cached particle mass
      G4double    m_Mass;

      //! Cached charge-squared (in units of e)
      G4double    m_ChargeSq;

      //! Cached particle kinetic energy
      G4double    m_KineticEnergy;

      //! Cached particle's gamma value
      G4double    m_Gamma;

      //! Cached particle's beta*gamma squared
      G4double    m_BetaGammaSq;

      //! Cached particle's beta squared
      G4double    m_BetaSq;

      //! Cached particle's maximum kinetic-energy loss
      G4double    m_Tmax;

      //! Step-length limit in units of range (0.01); not modifiable by user
      G4double    m_LinLossLimit;

      //! User's minimum kinetic energy for particles (default 1 MeV)
      G4double    m_UserTmin;

      //! User's maximum kinetic energy for particles (default 10 TeV)
      G4double    m_UserTmax;

      //! User's upper limit on maximum kinetic-energy loss (default infinity)
      G4double    m_UserMaxEnergyTransfer;

      //! Number of bins in each energy loss, fluctuation, and multiple-scattering table (70, fixed)
      G4int       m_Nbins;

      //! Number of materials in current geometry
      G4int       m_NMaterials;

      //! UNUSED Flag to indicate that Initialisation() method has been called
      G4bool      m_Initialised;
    };

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    //- inline G4double EnergyLossForExtrapolator::EnergyAfterStep(G4double kinEnergy,
    //-                                                            G4double step,
    //-                                                            const G4Material* mat,
    //-                                                            const G4String& name)
    //- {
    //-   return EnergyAfterStep(kinEnergy, step, mat, FindParticle(name));
    //- }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    //- inline G4double EnergyLossForExtrapolator::EnergyBeforeStep(G4double kinEnergy,
    //-                                                             G4double step,
    //-                                                             const G4Material* mat,
    //-                                                             const G4String& name)
    //- {
    //-   return EnergyBeforeStep(kinEnergy, step, mat, FindParticle(name));
    //- }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    //- inline G4double EnergyLossForExtrapolator::AverageScatteringAngle(G4double kinEnergy,
    //-     G4double step,
    //-     const G4Material* mat,
    //-     const G4String& name)
    //- {
    //-   return AverageScatteringAngle(kinEnergy, step, mat, FindParticle(name));
    //- }
    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    //- inline G4double
    //- EnergyLossForExtrapolator::EnergyDispersion(G4double kinEnergy,
    //-                                             G4double step,
    //-                                             const G4Material* mat,
    //-                                             const G4String& name)
    //- {
    //-   return EnergyDispersion(kinEnergy, step, mat, FindParticle(name));
    //- }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double
    EnergyLossForExtrapolator::AverageScatteringAngle(G4double kinEnergy,
                                                      G4double stepLength,
                                                      const G4Material* mat,
                                                      const G4ParticleDefinition* part)
    {
      G4double theta = 0.0;
      if (SetupKinematics(part, mat, kinEnergy)) {
        G4double t = stepLength / m_RadLength;
        G4double y = std::max(0.001, t);
        theta = 19.23 * CLHEP::MeV * std::sqrt(m_ChargeSq * t) * (1.0 + 0.038 * std::log(y)) / (m_BetaSq * m_Gamma * m_Mass);
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

    //- inline
    //- G4double EnergyLossForExtrapolator::EnergyDispersion(G4double kinEnergy,
    //-                                                      G4double stepLength,
    //-                                                      const G4Material* mat,
    //-                                                      const G4ParticleDefinition* part)
    //- {
    //-   G4double sig2 = 0.0;
    //-   if (SetupKinematics(part, mat, kinEnergy)) {
    //-     G4double step = ComputeTrueStep(mat, part, kinEnergy, stepLength);
    //-     sig2 = (1.0 / beta2 - 0.5) * CLHEP::twopi_mc2_rcl2 * tmax * step * electronDensity * charge2;
    //-   }
    //-   return sig2;
    //- }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline G4double EnergyLossForExtrapolator::ComputeValue(G4double x,
                                                            const G4PhysicsTable* table)
    {
      G4double res = 0.0;
      G4bool b;
      if (table) res = ((*table)[m_MaterialIndex])->GetValue(x, b);
      return res;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    //- inline void EnergyLossForExtrapolator::SetVerbose(G4int val)
    //- {
    //-   verbose = val;
    //- }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetMinKinEnergy(G4double val)
    {
      m_UserTmin = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetMaxKinEnergy(G4double val)
    {
      m_UserTmax = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

    inline void EnergyLossForExtrapolator::SetMaxEnergyTransfer(G4double val)
    {
      m_UserMaxEnergyTransfer = val;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* ENERGYLOSSFOREXTRAPOLATOR_H_ */
