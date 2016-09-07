/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <skim/softwaretrigger/core/utilities.h>
#include <TRandom.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    double getThetaOfNegativeTrackWithMaximumRhoInEvent()
    {
      StoreObjPtr<ParticleList> pionParticles("pion+:hlt");

      if (pionParticles.isValid()) {
        const Particle* trackWithMaximumRho = getElementWithMaximumRho<Particle>(pionParticles);
        if (trackWithMaximumRho) {
          if (trackWithMaximumRho->getCharge() == -1) {
            return trackWithMaximumRho->getMomentum().Theta();
          } else {
            const Particle* trackWithSecondMaximumRho = getElementWithMaximumRhoBelow<Particle>(pionParticles,
                                                        getRho(trackWithMaximumRho));

            if (trackWithSecondMaximumRho and trackWithSecondMaximumRho->getCharge() == -1) {
              return trackWithSecondMaximumRho->getMomentum().Theta();
            }
          }
        }
      }

      return -1;
    }

    const ECLCluster* getECLCluster(const Particle& particle, const bool fromTrack)
    {
      if (fromTrack) {
        return particle.getTrack()->getRelated<ECLCluster>();
      } else {
        return particle.getECLCluster();
      }
    }

    double getRhoOfECLClusterWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& pions,
                                                 const StoreObjPtr<ParticleList>& gammas,
                                                 const double belowLimit)
    {
      const ECLCluster* eclClusterWithMaximumRho = nullptr;
      double maximumRho = -1.;

      for (const Particle& particle : *pions) {
        const ECLCluster* tmpCluster = getECLCluster(particle, true);
        if (not tmpCluster) {
          continue;
        }

        const double& currentRho = getRho(tmpCluster);

        if (currentRho >= belowLimit) {
          continue;
        }

        if (currentRho > maximumRho) {
          maximumRho = currentRho;
          eclClusterWithMaximumRho = tmpCluster;
        }
      }

      for (const Particle& particle : *gammas) {
        const ECLCluster* tmpCluster = getECLCluster(particle, false);
        if (not tmpCluster) {
          continue;
        }

        const double& currentRho = getRho(tmpCluster);

        if (currentRho >= belowLimit) {
          continue;
        }

        if (currentRho > maximumRho) {
          maximumRho = currentRho;
          eclClusterWithMaximumRho = tmpCluster;
        }
      }

      return maximumRho;
    }

    bool makePreScale(const unsigned int& preScaleFactor)
    {
      // A prescale factor of one is always true...
      if (preScaleFactor == 1) {
        return true;
        // ... and a prescale factor of 0 is always false...
      } else if (preScaleFactor == 0) {
        return false;
      } else {
        // All other cases are a bit more interesting
        // We do this by drawing a random number between 0 and m_preScaleFactor - 1 and comparing it to 0.
        // The probability to get back a true result is then given by 1/m_preScaleFactor.
        const unsigned int randomNumber = gRandom->Integer(preScaleFactor);
        return randomNumber == 0;
      }
    }

    bool makePreScale(const std::vector<unsigned int>& preScaleFactors)
    {
      if (preScaleFactors.empty()) {
        return false;
      }

      const unsigned int factorSize = preScaleFactors.size();

      if (factorSize == 1) {
        return makePreScale(preScaleFactors.front());
      } else {
        const double& theta = getThetaOfNegativeTrackWithMaximumRhoInEvent();
        const double& intervalSize = TMath::Pi() / factorSize;

        B2ASSERT("Theta is negative, something went truly wrong here", theta >= 0);

        const unsigned int index = static_cast<unsigned int>(std::floor(theta / intervalSize));

        B2ASSERT("Index is larger than pre scale factor array.", index >= factorSize);

        return makePreScale(preScaleFactors[index]);
      }
    }
  }
}