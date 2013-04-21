/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTENERGYLOSS_H_
#define EXTENERGYLOSS_H_

#include <globals.hh>
#include <G4VContinuousProcess.hh>

class G4ParticleDefinition;
class G4Track;
class G4Step;

namespace Belle2 {

  namespace Simulation {

    class EnergyLossForExtrapolator;

    /**
     * Define physics process to calculate energy loss without fluctuations
     */
    class ExtEnergyLoss: public G4VContinuousProcess {

    public:

      //! constructor
      ExtEnergyLoss(const G4String& processName = "ExtEnergyLoss", G4ProcessType aType = fElectromagnetic);

      //! destructor
      virtual ~ExtEnergyLoss();

      //! Reports if this process is applicable to the particle being swum
      inline G4bool IsApplicable(const G4ParticleDefinition&);

      //! Gets step limit for the particle being swum
      G4double GetContinuousStepLimit(const G4Track&, G4double, G4double, G4double&);

      //! Apply energy loss process along the step
      G4VParticleChange* AlongStepDoIt(const G4Track&, const G4Step&);

      //! Gets the step limit for this process
      inline G4double GetStepLimit();

      //! Sets the step limit for this process
      inline void SetStepLimit(G4double);

    private:

      //! Pointer to the geant4e-specific energy-loss and mult-scat class
      EnergyLossForExtrapolator* m_energyLossForExtrapolator;

      //! Step limit for this process (fraction of KE that could be lost in one step)
      G4double m_StepLimit;

    };

    inline G4bool ExtEnergyLoss::IsApplicable(const G4ParticleDefinition& aParticleType)
    {
      return (aParticleType.GetPDGCharge() != 0); // this process applies to all charged particles
    }

    inline G4double ExtEnergyLoss::GetStepLimit()
    {
      return m_StepLimit;
    }

    inline void ExtEnergyLoss::SetStepLimit(G4double val)
    {
      m_StepLimit = val;
    }

  } //end namespace Simulation

} //end Belle2 namespace

#endif /* EXTENERGYLOSS_H_ */
