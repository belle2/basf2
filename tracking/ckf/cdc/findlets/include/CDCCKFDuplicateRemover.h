/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun, Simon Kurz                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/ckf/cdc/entities/CDCCKFResult.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>
#include <tracking/ckf/cdc/filters/paths/CDCPathFilterFactory.h>

#include <tracking/ckf/cdc/filters/pathPairs/CDCPathPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>


namespace Belle2 {
  /// Remove duplicate paths created from ECLShowers
  /// These typically come from the seeding with two charge assumptions and Bremsstrahlung
  class CDCCKFDuplicateRemover : public TrackFindingCDC::Findlet<CDCCKFResult> {
  public:
    CDCCKFDuplicateRemover()
    {
      addProcessingSignalListener(&m_filter_badTracks);
      addProcessingSignalListener(&m_filter_duplicateTrack);
      addProcessingSignalListener(&m_filter_duplicateSeed);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      m_filter_badTracks.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("badTracks", prefix));
      m_filter_duplicateTrack.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("duplicateTrack", prefix));
      m_filter_duplicateSeed.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("duplicateSeed", prefix));
    }

    /// main method of the findlet, merges and filters paths
    void apply(std::vector<CDCCKFResult>& results) override
    {
      B2DEBUG(100, "CDCCKFDuplicateRemover: " << results.size() << " paths created (might be without any hits)");

      std::vector<CDCCKFResult> goodResults;

      // Additional filter (typically check if charge of reconstructed track is equal to charge of seed)
      TrackFindingCDC::Weight weight;
      std::unordered_map<double, CDCCKFResult> resultToWeightList;
      for (const auto& result : results) {
        weight = m_filter_badTracks(result);
        if (not std::isnan(weight)) {
          goodResults.push_back(result);
        }
      }

      int n_goodresults = goodResults.size();

      B2DEBUG(100, "CDCCKFDuplicateRemover: " << goodResults.size() << " paths created (after filtering)");

      if (n_goodresults > 1) {
        for (const auto& result : goodResults) {
          B2DEBUG(100, "charge = " << result.front().getSeed()->getChargeSeed() << "; "
                  << "theta = " << result.front().getSeed()->getPositionSeed().Theta() * 180. / M_PI << "; "
                  << (result.size() - 1) << " hits (" << result.at(1).getWireHit()->getWire().getICLayer() << "->" <<
                  result.back().getWireHit()->getWire().getICLayer() << "); "
                  << "r/z = " << result.front().getSeed()->getPositionSeed().Perp() << "; " << result.front().getSeed()->getPositionSeed().Z());
        }
      }

      // If both charge assumptions lead to a good track, only pick one of the to avoid duplicate tracks
      auto iter = goodResults.begin();
      while (iter < goodResults.end()) {
        double rClus = iter->front().getSeed()->getPositionSeed().Perp();
        double zClus = iter->front().getSeed()->getPositionSeed().Z();

        auto iter2 = iter + 1;
        bool increaseIter = true;
        while (iter2 < goodResults.end()) {
          // find tracks from same seed
          if (std::abs(iter2->front().getSeed()->getPositionSeed().Perp() - rClus) < 0.000001
              && std::abs(iter2->front().getSeed()->getPositionSeed().Z() - zClus) < 0.000001) {
            // let filter decide which one to keep
            bool selectFirst = m_filter_duplicateTrack({&*iter, &*iter2});
            if (selectFirst) {
              iter2 = goodResults.erase(iter2);
            } else {
              iter = goodResults.erase(iter);
              increaseIter = false;
              break;
            }
          } else {
            ++iter2;
          }
        }
        if (increaseIter) {
          ++iter;
        }
      }

      B2DEBUG(100, "CDCCKFDuplicateRemover: " << goodResults.size() << " paths created (after duplicates)");

      if (n_goodresults > 1) {
        for (const auto& result : goodResults) {
          B2DEBUG(100, "charge = " << result.front().getSeed()->getChargeSeed() << "; "
                  << "theta = " << result.front().getSeed()->getPositionSeed().Theta() * 180. / M_PI << "; "
                  << (result.size() - 1) << " hits (" << result.at(1).getWireHit()->getWire().getICLayer() << "->" <<
                  result.back().getWireHit()->getWire().getICLayer() << "); "
                  << "r/z = " << result.front().getSeed()->getPositionSeed().Perp() << "; " << result.front().getSeed()->getPositionSeed().Z());
        }
      }

      // Remove duplicate tracks from Bremsstrahlung
      // Be careful as this might also remove photon conversions (m_filter_duplicateSeed decides if both should be kept)
      iter = goodResults.begin();
      while (iter < goodResults.end()) {
        double phiClus = iter->front().getSeed()->getPositionSeed().Phi();
        double thetaClus = iter->front().getSeed()->getPositionSeed().Theta();

        auto iter2 = iter + 1;
        bool increaseIter = true;
        while (iter2 < goodResults.end()) {
          // find tracks from close-by seeds
          if (std::abs(TVector2::Phi_mpi_pi(iter2->front().getSeed()->getPositionSeed().Phi() - phiClus)) < 2.
              && std::abs(iter2->front().getSeed()->getPositionSeed().Theta() - thetaClus) < 0.1) {
            // let filter decide which one to keep
            bool isDuplicate = m_filter_duplicateSeed({&*iter, &*iter2});
            if (! isDuplicate) {
              B2DEBUG(100, "Keeping both tracks");
              ++iter2;
            } else {
              B2DEBUG(100, "Duplicate hits found");
              bool selectFirst = m_filter_duplicateTrack({&*iter, &*iter2});
              if (selectFirst) {
                iter2 = goodResults.erase(iter2);
              } else {
                iter = goodResults.erase(iter);
                increaseIter = false;
                break;
              }
            }
          } else {
            ++iter2;
          }
        }
        if (increaseIter) {
          ++iter;
        }
      }

      results = goodResults;

      B2DEBUG(100, "CDCCKFDuplicateRemover: " << results.size() << " paths created (after merging)");

      if (n_goodresults > 1) {
        for (const auto& result : results) {
          B2DEBUG(100, "charge = " << result.front().getSeed()->getChargeSeed() << "; "
                  << "theta = " << result.front().getSeed()->getPositionSeed().Theta() * 180. / M_PI << "; "
                  << (result.size() - 1) << " hits (" << result.at(1).getWireHit()->getWire().getICLayer() << "->" <<
                  result.back().getWireHit()->getWire().getICLayer() << "); "
                  << "r/z = " << result.front().getSeed()->getPositionSeed().Perp() << "; " << result.front().getSeed()->getPositionSeed().Z());
        }
      }
    }

  private:
    /// Filter  to remove badly reconstructed tracks (e.g. wrongly assigned charge)
    TrackFindingCDC::ChooseableFilter<CDCPathFilterFactory> m_filter_badTracks;
    /// Filter to remove duplicates from helix extrapolation (2 charge assumptions)
    TrackFindingCDC::ChooseableFilter<CDCPathPairFilterFactory> m_filter_duplicateTrack;
    /// Merge duplicate paths (mostly seeds from Bremstrahlung)
    TrackFindingCDC::ChooseableFilter<CDCPathPairFilterFactory> m_filter_duplicateSeed;
  };
}
