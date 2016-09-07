/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/calculations/HLTCalculator.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
// TODO: Also cache it
#include <analysis/utility/PCmsLabTransform.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    void HLTCalculator::requireStoreArrays()
    {
      m_pionParticles.isRequired();
      m_gammaParticles.isRequired();
    };

    template <class AnEntityType>
    const AnEntityType* getElementFromParticle(const Particle& particle);

    template <>
    const Particle* getElementFromParticle(const Particle& particle)
    {
      return &particle;
    }

    template <>
    const ECLCluster* getElementFromParticle(const Particle& particle)
    {
      return particle.getECLCluster();
    }

    template <class AnEntity>
    double getRho(const AnEntity* entity)
    {
      if (not entity) {
        return -1;
      }
      const TLorentzVector& fourVector = entity->get4Vector();
      return PCmsLabTransform::labToCms(fourVector).Rho();
    }

    const ECLCluster* getECLCluster(const Particle& particle, const bool fromTrack)
    {
      if (fromTrack) {
        return particle.getTrack()->getRelated<ECLCluster>();
      } else {
        return particle.getECLCluster();
      }
    }

    template <class AReturnType>
    const AReturnType* getElementWithMaximumRhoBelow(const StoreObjPtr<ParticleList>& particles,
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

    template <class AReturnType>
    const AReturnType* getElementWithMaximumRho(const StoreObjPtr<ParticleList>& particles)
    {
      return getElementWithMaximumRhoBelow<AReturnType>(particles, std::nan(""));
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

    double getRhoOfECLClusterWithMaximumRho(const StoreObjPtr<ParticleList>& pionshlt,
                                            const StoreObjPtr<ParticleList>& gammahlt)
    {
      return getRhoOfECLClusterWithMaximumRhoBelow(pionshlt, gammahlt, std::nan(""));
    }

    void HLTCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {
      // Prefetch some later needed objects/values
      const ECLCluster* eclClusterWithMaximumRho = getElementWithMaximumRho<ECLCluster>(m_gammaParticles);
      const Particle* trackWithMaximumRho = getElementWithMaximumRho<Particle>(m_pionParticles);
      const Particle* trackWithSecondMaximumRho = getElementWithMaximumRhoBelow<Particle>(m_pionParticles,
                                                  getRho(trackWithMaximumRho));

      const double& rhoOfECLClusterWithMaximumRho = getRhoOfECLClusterWithMaximumRho(m_pionParticles, m_gammaParticles);
      const double& rhoOfECLClusterWithSecondMaximumRho = getRhoOfECLClusterWithMaximumRhoBelow(m_pionParticles,
                                                          m_gammaParticles,
                                                          rhoOfECLClusterWithMaximumRho);

      const double& rhoOfTrackWithMaximumRho = getRho(trackWithMaximumRho);
      const double& rhoOfTrackWithSecondMaximumRho = getRho(trackWithSecondMaximumRho);

      // Simple to calculate variables
      // EC1CMSLE
      calculationResult["EC1CMSLE"] = rhoOfECLClusterWithMaximumRho;

      // EC2CMSLE
      calculationResult["EC2CMSLE"] = rhoOfECLClusterWithSecondMaximumRho;

      // EC12CMSLE
      calculationResult["EC12CMSLE"] = rhoOfECLClusterWithMaximumRho + rhoOfECLClusterWithSecondMaximumRho;

      // nTracksLE
      calculationResult["nTracksLE"] = m_pionParticles->getListSize();

      // P1CMSBhabhaLE
      calculationResult["P1CMSBhabhaLE"] = rhoOfTrackWithMaximumRho;

      // P2CMSBhabhaLE
      calculationResult["P2CMSBhabhaLE"] = rhoOfTrackWithSecondMaximumRho;

      // P12CMSBhabhaLE
      calculationResult["P12CMSBhabhaLE"] = rhoOfTrackWithMaximumRho + rhoOfTrackWithSecondMaximumRho;

      // Medium hard to calculate variables
      // ENeutralLE
      if (eclClusterWithMaximumRho) {
        calculationResult["ENeutralLE"] = eclClusterWithMaximumRho->getEnergy();
      } else {
        calculationResult["ENeutralLE"] = -1;
      }

      // nECLMatchTracksLE
      const unsigned int numberOfTracksWithECLMatch = std::count_if(m_pionParticles->begin(), m_pionParticles->end(),
      [](const Particle & particle) {
        return getECLCluster(particle, true) != nullptr;
      });
      calculationResult["nECLMatchTracksLE"] = numberOfTracksWithECLMatch;


      // AngleGTLE
      double angleGTLE = -10.;
      if (eclClusterWithMaximumRho) {
        const TLorentzVector& V4g1 = eclClusterWithMaximumRho->get4Vector();
        if (trackWithMaximumRho) {
          const TLorentzVector& V4p1 = trackWithMaximumRho->get4Vector();
          const double theta1 = (V4g1.Vect()).Angle(V4p1.Vect());
          if (angleGTLE < theta1) angleGTLE = theta1;
        }
        if (trackWithSecondMaximumRho) {
          const TLorentzVector& V4p2 = trackWithSecondMaximumRho->get4Vector();
          const double theta2 = (V4g1.Vect()).Angle(V4p2.Vect());
          if (angleGTLE < theta2) angleGTLE = theta2;
        }
      }
      calculationResult["AngleGTLE"] = angleGTLE;


      // maxAngleTTLE
      double maxAngleTTLE = -10.;
      if (m_pionParticles->getListSize() >= 2) {
        for (unsigned int i = 0; i < m_pionParticles->getListSize() - 1; i++) {
          Particle* par1 = m_pionParticles->getParticle(i);
          for (unsigned int j = i + 1; j < m_pionParticles->getListSize(); j++) {
            Particle* par2 = m_pionParticles->getParticle(j);
            TLorentzVector V4p1 = par1->get4Vector();
            TLorentzVector V4p2 = par2->get4Vector();
            const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
            const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
            const double temp = V3p1.Angle(V3p2);
            if (maxAngleTTLE < temp) maxAngleTTLE = temp;
          }
        }
      }

      calculationResult["maxAngleTTLE"] = maxAngleTTLE;

      // nEidLE
      const unsigned int nEidLE = std::count_if(m_pionParticles->begin(), m_pionParticles->end(), [](const Particle & p) {
        const double& momentum  = p.getMomentumMagnitude();
        const double& rho = getRho(&p);
        const ECLCluster* eclTrack = getECLCluster(p, true);
        if (eclTrack) {
          const double& energyOverMomentum = eclTrack->getEnergy() / momentum;
          return rho > 5.0 && energyOverMomentum > 0.8;
        }
        return false;
      });

      calculationResult["nEidLE"] = nEidLE;


      // VisibleEnergyLE
      const double visibleEnergyTracks = std::accumulate(m_pionParticles->begin(), m_pionParticles->end(), 0.0,
      [](const double & visibleEnergy, const Particle & p) {
        return visibleEnergy + p.getMomentumMagnitude();
      });

      const double visibleEnergyGammas = std::accumulate(m_pionParticles->begin(), m_pionParticles->end(), 0.0,
      [](const double & visibleEnergy, const Particle & p) {
        return visibleEnergy + p.getMomentumMagnitude();
      });

      calculationResult["VisibleEnergyLE"] = visibleEnergyTracks + visibleEnergyGammas;

      // EtotLE
      const double eTotTracks = std::accumulate(m_pionParticles->begin(), m_pionParticles->end(), 0.0,
      [](const double & eTot, const Particle & p) {
        const ECLCluster* eclCluster = getECLCluster(p, true);
        if (eclCluster) {
          const double eclEnergy = eclCluster->getEnergy();
          if (eclEnergy > 0.1) {
            return eTot + eclCluster->getEnergy();
          }
        }
        return eTot;
      });

      const double eTotGammas = std::accumulate(m_gammaParticles->begin(), m_gammaParticles->end(), 0.0,
      [](const double & eTot, const Particle & p) {
        return eTot + p.getEnergy();
      });

      calculationResult["EtotLE"] = eTotTracks + eTotGammas;
    }
  }
}
