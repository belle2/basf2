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
#include <hlt/softwaretrigger/calculations/FastRecoCalculator.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <TVector3.h>
#include <numeric>

namespace Belle2 {
  namespace SoftwareTrigger {
    void FastRecoCalculator::requireStoreArrays()
    {
      // The CDC reco tracks should either be in the store array "CDCRecoTracks" or in the normal "RecoTracks".
      if (not m_cdcRecoTracks.isOptional()) {
        m_recoTracks.isRequired();
      }
      m_eclClusters.isRequired();
    };

    void FastRecoCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
    {
      StoreArray<RecoTrack> recoTracks;
      if (m_cdcRecoTracks.isValid()) {
        recoTracks = m_cdcRecoTracks;
      } else {
        recoTracks = m_recoTracks;
      }

      // We only want to access the photon ECL clusters. We use references to not need to copy always.
      m_eclClustersWithPhotonHypothesis.clear();
      for (const ECLCluster& eclCluster : m_eclClusters) {
        if (eclCluster.getHypothesisId() == ECLCluster::Hypothesis::c_nPhotons) {
          m_eclClustersWithPhotonHypothesis.emplace_back(eclCluster);
        }
      }

      // Prepare some cache objects.
      std::vector<TVector3> momenta;
      momenta.reserve(static_cast<unsigned long>(recoTracks.getEntries()));

      for (const RecoTrack& cdcRecoTrack : recoTracks) {
        momenta.push_back(cdcRecoTrack.getMomentumSeed());
      }

      std::vector<double> sortedECLEnergies;
      sortedECLEnergies.reserve(static_cast<unsigned long>(m_eclClustersWithPhotonHypothesis.size()));

      for (const ECLCluster& eclCluster : m_eclClustersWithPhotonHypothesis) {
        sortedECLEnergies.push_back(eclCluster.getEnergy());
      }

      std::sort(sortedECLEnergies.begin(), sortedECLEnergies.end(), std::greater<double>());

      // TODO: Do only return the first (we do not need more).
      const std::vector<double>& sortedRhoECLEnergyList =
        getSortedEnergiesFrom(std::begin(m_eclClustersWithPhotonHypothesis),
                              std::end(m_eclClustersWithPhotonHypothesis), m_transformer);
      const std::vector<double>& sortedRhoCDCEnergyList =
        getSortedEnergiesFrom(std::begin(recoTracks),
                              std::end(recoTracks), m_transformer);

      // Calculate the visible energy
      double visibleEnergy = 0;
      if (not momenta.empty()) {
        visibleEnergy = std::accumulate(momenta.begin(), momenta.end(), visibleEnergy,
        [](const double & value, const TVector3 & momentum) {
          return value + momentum.Mag();
        });
      }
      if (not m_eclClustersWithPhotonHypothesis.empty()) {
        visibleEnergy = std::accumulate(std::begin(m_eclClustersWithPhotonHypothesis),
                                        std::end(m_eclClustersWithPhotonHypothesis), visibleEnergy,
        [](const double & value, const ECLCluster & eclCluster) {
          ClusterUtils C;
          return value + C.Get4MomentumFromCluster(&eclCluster).Vect().Mag();
        });
      }

      calculationResult["visible_energy"] = visibleEnergy;

      calculationResult["energy_sum_of_high_energy_ecl"] =
        std::accumulate(std::begin(m_eclClustersWithPhotonHypothesis),
                        std::end(m_eclClustersWithPhotonHypothesis), static_cast<double>(0.0),
      [](const double & value, const ECLCluster & eclCluster) {
        const double& energy = eclCluster.getEnergy();
        if (energy > 0.05) {
          return value + energy;
        } else {
          return value;
        }
      });
      calculationResult["energy_sum_of_ecl"] =
        std::accumulate(std::begin(m_eclClustersWithPhotonHypothesis),
                        std::end(m_eclClustersWithPhotonHypothesis), static_cast<double>(0.0),
      [](const double & value, const ECLCluster & eclCluster) {
        return value + eclCluster.getEnergy();
      });

      calculationResult["number_of_high_energy_ecl_clusters"] =
        std::count_if(std::begin(m_eclClustersWithPhotonHypothesis),
                      std::end(m_eclClustersWithPhotonHypothesis),
      [](const ECLCluster & eclCluster) {
        return eclCluster.getEnergy() > 0.05;
      });

      calculationResult["number_of_cdc_tracks"] = momenta.size();
      calculationResult["number_of_ecl_cluster"] = m_eclClustersWithPhotonHypothesis.size();

      calculationResult["highest_1_ecl"] = 0;
      calculationResult["highest_2_ecl"] = 0;
      calculationResult["highest_3_ecl"] = 0;

      if (sortedECLEnergies.size() > 0) {
        calculationResult["highest_1_ecl"] = sortedECLEnergies[0];

        if (sortedECLEnergies.size() > 1) {
          calculationResult["highest_2_ecl"] = sortedECLEnergies[0] + sortedECLEnergies[1];

          if (sortedECLEnergies.size() > 2) {
            calculationResult["highest_3_ecl"] = sortedECLEnergies[0] + sortedECLEnergies[1] + sortedECLEnergies[2];
          }
        }
      }

      calculationResult["highest_1_ecl_rho"] = 0;
      calculationResult["highest_2_ecl_rho"] = 0;
      calculationResult["highest_3_ecl_rho"] = 0;

      if (sortedRhoECLEnergyList.size() > 0) {
        calculationResult["highest_1_ecl_rho"] = sortedRhoECLEnergyList[0];

        if (sortedRhoECLEnergyList.size() > 1) {
          calculationResult["highest_2_ecl_rho"] = sortedRhoECLEnergyList[0] + sortedRhoECLEnergyList[1];

          if (sortedRhoECLEnergyList.size() > 2) {
            calculationResult["highest_3_ecl_rho"] = sortedRhoECLEnergyList[0] + sortedRhoECLEnergyList[1] + sortedRhoECLEnergyList[2];
          }
        }
      }

      if (momenta.empty()) {
        calculationResult["max_pt"] = 0;
        calculationResult["max_pz"] = 0;
        calculationResult["number_of_high_energy_tracks"] = 0;
        calculationResult["mean_abs_theta"] = 0;
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

        calculationResult["mean_abs_theta"] = std::accumulate(momenta.begin(), momenta.end(), static_cast<double>(0),
        [](const double & value, const TVector3 & momentum) {
          return value + momentum.Theta();
        }) / momenta.size();
      }

      calculationResult["highest_1_cdc_rho"] = 0;
      calculationResult["highest_2_cdc_rho"] = 0;
      calculationResult["highest_3_cdc_rho"] = 0;

      if (sortedRhoCDCEnergyList.size() > 0) {
        calculationResult["highest_1_cdc_rho"] = sortedRhoCDCEnergyList[0];

        if (sortedRhoCDCEnergyList.size() > 1) {
          calculationResult["highest_2_cdc_rho"] = sortedRhoCDCEnergyList[0] + sortedRhoCDCEnergyList[1];

          if (sortedRhoCDCEnergyList.size() > 2) {
            calculationResult["highest_3_cdc_rho"] = sortedRhoCDCEnergyList[0] + sortedRhoCDCEnergyList[1] +
                                                     sortedRhoCDCEnergyList[2];
          }
        }
      }
    }
  }
}
