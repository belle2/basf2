/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <TLorentzVector.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper function to extract a four vector out of an entity.
    template<class T>
    inline TLorentzVector getFourVector(const T& item);

    /// Helper function to extract a four vector out of an ECLCluster by combining the momentum information.
    template<>
    inline TLorentzVector getFourVector(const std::reference_wrapper<const ECLCluster>& cluster)
    {
      ClusterUtils C;
      const TLorentzVector& v = C.Get4MomentumFromCluster(&(cluster.get()));
      return TLorentzVector(v.Px(), v.Py(), v.Pz(), v.E());
    }

    /// Helper function to extract a four vector out of an ECLCluster by combining the momentum information and the pion mass.
    template<>
    inline TLorentzVector getFourVector(const RecoTrack& track)
    {
      const TVector3& positionSeed = track.getPositionSeed();
      return TLorentzVector(positionSeed.X(), positionSeed.Y(), positionSeed.Z(),
                            sqrt(positionSeed.Mag2() + Const::pionMass * Const::pionMass));
    }

    /// Helper function to sort the entries in a store array by their energy.
    template<class AListIterator>
    static std::vector<double> getSortedEnergiesFrom(const AListIterator& begin, const AListIterator& end,
                                                     const PCmsLabTransform& transformer)
    {
      std::vector<TLorentzVector> lorentzVectors;
      lorentzVectors.reserve(std::distance(begin, end));

      std::for_each(begin, end, [&](const auto & item) {
        const TLorentzVector& fourVector = getFourVector(item);
        lorentzVectors.push_back(transformer.rotateLabToCms() * fourVector);
      });

      std::sort(lorentzVectors.begin(), lorentzVectors.end(),
      [](const TLorentzVector & lhs, const TLorentzVector & rhs) {
        return lhs.Rho() > rhs.Rho();
      });

      std::vector<double> energies;
      energies.reserve(lorentzVectors.size());

      for (const TLorentzVector& lorentzVector : lorentzVectors) {
        energies.push_back(lorentzVector.Energy());
      }

      return energies;
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

      const TLorentzVector& fourVector = entity->get4Vector();
      return PCmsLabTransform::labToCms(fourVector).Rho();
    }

    // Template specialization for ECLCluster
    template<>
    inline double getRho(const ECLCluster* entity)
    {
      if (not entity) {
        return -1;
      }

      ClusterUtils C;
      return PCmsLabTransform::labToCms(C.Get4MomentumFromCluster(entity)).Rho();
    }

    /// Helper function to extract the ECL cluster from a particle - either directly or via the attached track.
    extern const ECLCluster* getECLCluster(const Particle& particle, const bool fromTrack);

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
