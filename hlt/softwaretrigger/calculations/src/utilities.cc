/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/calculations/utilities.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    const ECLCluster* getECLCluster(const Particle& particle)
    {
      return particle.getECLCluster();
    }

    double getRhoOfECLClusterWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& pions,
                                                 const StoreObjPtr<ParticleList>& gammas,
                                                 const double belowLimit)
    {
      double maximumRho = -1.;

      for (const Particle& particle : *pions) {
        const ECLCluster* tmpCluster = getECLCluster(particle);
        if (not tmpCluster) {
          continue;
        }

        const double& currentRho = getRho(tmpCluster);

        if (currentRho >= belowLimit) {
          continue;
        }

        if (currentRho > maximumRho) {
          maximumRho = currentRho;
        }
      }

      for (const Particle& particle : *gammas) {
        const ECLCluster* tmpCluster = getECLCluster(particle);
        if (not tmpCluster) {
          continue;
        }

        const double& currentRho = getRho(tmpCluster);

        if (currentRho >= belowLimit) {
          continue;
        }

        if (currentRho > maximumRho) {
          maximumRho = currentRho;
        }
      }

      return maximumRho;
    }
  }
}
