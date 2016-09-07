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
#include <TLorentzVector.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper function to do a prescaling using a random integer number and the prescaling factor from the object.
    extern bool makePreScale(const unsigned int& preScaleFactor);

    extern bool makePreScale(const std::vector<unsigned int>& preScaleFactors);

    template<class T>
    inline TLorentzVector getFourVector(const T& item);

    template<>
    inline TLorentzVector getFourVector(const ECLCluster& cluster)
    {
      return TLorentzVector(cluster.getPx(), cluster.getPy(), cluster.getPz(), cluster.getEnergy());
    }

    template<>
    inline TLorentzVector getFourVector(const RecoTrack& cluster)
    {
      const TVector3& positionSeed = cluster.getPositionSeed();
      return TLorentzVector(positionSeed.X(), positionSeed.Y(), positionSeed.Z(),
                            sqrt(positionSeed.Mag2() + Const::pionMass * Const::pionMass));
    }

    template<class T>
    static std::vector<double> getSortedEnergiesFrom(const StoreArray<T>& storeArray, const PCmsLabTransform& transformer)
    {
      std::vector<TLorentzVector> lorentzVectors;
      lorentzVectors.reserve(storeArray.getEntries());

      for (const T& item : storeArray) {
        const TLorentzVector& fourVector = getFourVector(item);
        lorentzVectors.push_back(transformer.rotateLabToCms() * fourVector);
      }

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

    template<class AnEntityType>
    inline const AnEntityType* getElementFromParticle(const Particle& particle);

    template<>
    inline const Particle* getElementFromParticle(const Particle& particle)
    {
      return &particle;
    }

    template<>
    inline const ECLCluster* getElementFromParticle(const Particle& particle)
    {
      return particle.getECLCluster();
    }

    template<class AnEntity>
    inline double getRho(const AnEntity* entity)
    {
      if (not entity) {
        return -1;
      }
      const TLorentzVector& fourVector = entity->get4Vector();
      return PCmsLabTransform::labToCms(fourVector).Rho();
    }

    extern const ECLCluster* getECLCluster(const Particle& particle, const bool fromTrack);

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

    template<class AReturnType>
    inline const AReturnType* getElementWithMaximumRho(const StoreObjPtr<ParticleList>& particles)
    {
      return getElementWithMaximumRhoBelow<AReturnType>(particles, std::nan(""));
    }

    extern double getRhoOfECLClusterWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& pions,
                                                        const StoreObjPtr<ParticleList>& gammas,
                                                        const double belowLimit);

    inline double getRhoOfECLClusterWithMaximumRho(const StoreObjPtr<ParticleList>& pionshlt,
                                                   const StoreObjPtr<ParticleList>& gammahlt)
    {
      return getRhoOfECLClusterWithMaximumRhoBelow(pionshlt, gammahlt, std::nan(""));
    }
  }
}