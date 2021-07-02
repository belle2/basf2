/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <hlt/softwaretrigger/calculations/utilities.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    double getRhoOfECLClusterWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& pions,
                                                 const StoreObjPtr<ParticleList>& gammas,
                                                 const double belowLimit)
    {
      double maximumRho = -1.;

      for (const Particle& particle : *pions) {
        const ECLCluster* tmpCluster = particle.getECLCluster();
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
        const ECLCluster* tmpCluster = particle.getECLCluster();
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
