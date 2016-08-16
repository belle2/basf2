/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/calculations/FastRecoCalculator.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <TVector3.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    void FastRecoCalculator::requireStoreArrays()
    {
      m_cdcRecoTracks.isRequired("CDCRecoTracks");
      m_eclClusters.isRequired();
    };

    template <class T>
    TLorentzVector getFourVector(const T& item);

    template <>
    TLorentzVector getFourVector(const ECLCluster& cluster)
    {
      return TLorentzVector(cluster.getPx(), cluster.getPy(), cluster.getPz(), cluster.getEnergy());
    }

    template <>
    TLorentzVector getFourVector(const RecoTrack& cluster)
    {
      const TVector3& positionSeed = cluster.getPositionSeed();
      return TLorentzVector(positionSeed.X(), positionSeed.Y(), positionSeed.Z(),
                            sqrt(positionSeed.Mag2() + Const::pionMass * Const::pionMass));
    }

    template <class T>
    std::vector<double> getSortedEnergiesFrom(const StoreArray<T>& storeArray)
    {
      std::vector<TLorentzVector> lorentzVectors;
      lorentzVectors.reserve(storeArray.getEntries());

      for (const T& item : storeArray) {
        const TLorentzVector& fourVector = getFourVector(item);
        lorentzVectors.push_back(PCmsLabTransform::labToCms(fourVector));
      }

      std::sort(lorentzVectors.begin(), lorentzVectors.end(),
      [](const TLorentzVector & lhs, const TLorentzVector & rhs) {
        return lhs.Rho() < rhs.Rho();
      });

      std::vector<double> energies;
      energies.reserve(lorentzVectors.size());

      for (const TLorentzVector& lorentzVector : lorentzVectors) {
        energies.push_back(lorentzVector.Energy());
      }

      return energies;
    }

    void FastRecoCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {
      std::vector<TVector3> momenta;
      momenta.reserve(m_cdcRecoTracks.getEntries());

      for (const RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
        momenta.push_back(cdcRecoTrack.getMomentumSeed());
      }

      // TODO: Do only return the first (we do not need more).
      const std::vector<double>& sortedRhoECLEnergyList = getSortedEnergiesFrom(m_eclClusters);
      const std::vector<double>& sortedRhoCDCEnergyList = getSortedEnergiesFrom(m_cdcRecoTracks);

      double visibleEnergy = 0;
      if (not momenta.empty()) {
        visibleEnergy = std::accumulate(momenta.begin(), momenta.end(), visibleEnergy,
        [](const double & value, const TVector3 & momentum) {
          return value + momentum.Mag();
        });
      }
      if (m_eclClusters.getEntries() > 0) {
        visibleEnergy = std::accumulate(std::begin(m_eclClusters), std::end(m_eclClusters), visibleEnergy,
        [](const double & value, const ECLCluster & eclCluster) {
          return value + eclCluster.getMomentum().Mag();
        });
      }

      calculationResult["visible_energy"] = visibleEnergy;

      if (sortedRhoECLEnergyList.empty()) {
        calculationResult["highest_1_ecl"] = 0;
        calculationResult["energy_sum_of_high_energy_ecl"] = 0;
      } else {
        calculationResult["highest_1_ecl"] = sortedRhoECLEnergyList.front();
        calculationResult["energy_sum_of_high_energy_ecl"] = std::accumulate(std::begin(m_eclClusters), std::end(m_eclClusters), 0,
        [](const double & value, const ECLCluster & eclCluster) {
          const double& energy = eclCluster.getEnergy();
          if (energy > 0.05) {
            return value + energy;
          } else {
            return value;
          }
        });
      }


      calculationResult["number_of_cdc_tracks"] = momenta.size();
      calculationResult["number_of_ecl_cluster"] = m_eclClusters.getEntries();

      if (momenta.empty()) {
        calculationResult["max_pt"] = 0;
        calculationResult["max_pz"] = 0;
        calculationResult["number_of_high_energy_tracks"] = 0;
        calculationResult["mean_theta"] = 0;
      } else {
        calculationResult["max_pt"] = std::max_element(momenta.begin(), momenta.end(),
        [](const TVector3 & lhs, const TVector3 & rhs) {
          return rhs.Pt() > lhs.Pt();
        })->Pt();

        calculationResult["max_pz"] = std::max_element(momenta.begin(), momenta.end(),
        [](const TVector3 & lhs, const TVector3 & rhs) {
          return rhs.Z() > lhs.Z();
        })->Z();

        calculationResult["number_of_high_energy_tracks"] = std::count_if(momenta.begin(), momenta.end(),
        [](const TVector3 & momentum) {
          return momentum.Pt() > 0.2;
        });

        calculationResult["mean_theta"] = std::accumulate(momenta.begin(), momenta.end(), 0,
        [](const double & value, const TVector3 & momentum) {
          return value + momentum.Theta();
        }) / momenta.size();
      }

      if (sortedRhoCDCEnergyList.empty()) {
        calculationResult["first_highest_cdc_energies"] = 0;
      } else {
        calculationResult["first_highest_cdc_energies"] = sortedRhoCDCEnergyList.front();
      }
    }
  }
}
