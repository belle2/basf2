/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper function to extract a four vector out of an entity.
    template<class T>
    inline ROOT::Math::PxPyPzEVector getFourVector(const T& item);

    /// Helper function to extract a four vector out of an ECLCluster by combining the momentum information.
    template<>
    inline ROOT::Math::PxPyPzEVector getFourVector(const std::reference_wrapper<const ECLCluster>& cluster)
    {
      ClusterUtils C;
      const ROOT::Math::PxPyPzEVector& v = C.Get4MomentumFromCluster(&(cluster.get()), ECLCluster::EHypothesisBit::c_nPhotons);
      return ROOT::Math::PxPyPzEVector(v.Px(), v.Py(), v.Pz(), v.E());
    }

    /// Helper function to extract a four vector out of a RecoTrack by combining the momentum information and the pion mass.
    template<>
    inline ROOT::Math::PxPyPzEVector getFourVector(const RecoTrack& track)
    {
      const TVector3& positionSeed = track.getPositionSeed();
      return ROOT::Math::PxPyPzEVector(positionSeed.X(), positionSeed.Y(), positionSeed.Z(),
                                       sqrt(positionSeed.Mag2() + Const::pionMass * Const::pionMass));
    }

    /// Helper function to get "something" from a particle, where "something" depends on the different implementations.
    template<class AnEntityType>
    inline const AnEntityType* getElementFromParticle(const Particle& particle);

    /// Helper function to extract a particle from a particle (needed when the type is not know in templates).
    template<>
    inline const Particle* getElementFromParticle(const Particle& particle)
    {
      return &particle;
    }

    /// Helper function to extract an ECLCluster from a particle.
    template<>
    inline const ECLCluster* getElementFromParticle(const Particle& particle)
    {
      return particle.getECLCluster();
    }

    /// Helper function to extract Beam Energy in CMS frame.
    inline double BeamEnergyCMS()
    {
      PCmsLabTransform T;
      return T.getCMSEnergy() / 2.0;
    }

    /// Helper function to extract rho = momentum magnitude in the CMS frame from an entity (ECLCLuster or Particle).
    template<class AnEntity>
    inline double getRho(const AnEntity* entity)
    {
      if (not entity) {
        return -1;
      }

      const ROOT::Math::PxPyPzEVector& fourVector = entity->get4Vector();
      return PCmsLabTransform::labToCms(fourVector).P();
    }

    // Template specialization for ECLCluster
    template<>
    inline double getRho(const ECLCluster* entity)
    {
      if (not entity) {
        return -1;
      }

      ClusterUtils C;
      return PCmsLabTransform::labToCms(C.Get4MomentumFromCluster(entity, ECLCluster::EHypothesisBit::c_nPhotons)).P();
    }

    /**
     * Helper function to get the element with the maximal rho in the CMS frame in the particle list, which has a rho
     * value below a given threshold.
     */
    template<class AReturnType>
    static const AReturnType* getElementWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& particles,
                                                            const double belowLimit)
    {
      const AReturnType* elementMaximumRho = nullptr;
      double maximumRho = -1.;
      for (const Particle& currentParticle : *particles) {
        const auto currentElement = getElementFromParticle<AReturnType>(currentParticle);
        const double& currentRho = getRho(currentElement);
        if (currentRho >= belowLimit) {
          continue;
        }
        if (currentRho > maximumRho) {
          maximumRho = currentRho;
          elementMaximumRho = currentElement;
        }
      }
      return elementMaximumRho;
    }

    /// Helper function to get the element with the maximalrho in the CMS frame in a particle list.
    template<class AReturnType>
    inline const AReturnType* getElementWithMaximumRho(const StoreObjPtr<ParticleList>& particles)
    {
      return getElementWithMaximumRhoBelow<AReturnType>(particles, std::nan(""));
    }

    extern double getRhoOfECLClusterWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& pions,
                                                        const StoreObjPtr<ParticleList>& gammas,
                                                        const double belowLimit);

    /// Helper function to get the maximal rho value in CMS of the ECLCluster from either gammas or related to pions.
    inline double getRhoOfECLClusterWithMaximumRho(const StoreObjPtr<ParticleList>& pionshlt,
                                                   const StoreObjPtr<ParticleList>& gammahlt)
    {
      return getRhoOfECLClusterWithMaximumRhoBelow(pionshlt, gammahlt, std::nan(""));
    }
  }
}
