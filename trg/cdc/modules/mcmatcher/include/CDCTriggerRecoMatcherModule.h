/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <Eigen/Dense>

#include "framework/core/Module.h"
#include "framework/datastore/StoreArray.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "tracking/dataobjects/RecoTrack.h"
#include "cdc/dataobjects/CDCHit.h"
#include "trg/cdc/dataobjects/CDCTriggerTrack.h"
#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"

namespace Belle2 {

  namespace {
    template <typename Iter>
    struct IteratorRange {
      Iter first, last;
      IteratorRange(Iter f, Iter l) : first(f), last(l) {}
      Iter begin() const { return first; }
      Iter end() const { return last; }
    };

    template <typename Iter>
    inline IteratorRange<Iter> as_range(std::pair<Iter, Iter> const& p)
    {
      return IteratorRange<Iter>(p.first, p.second);
    }

    typedef int HitId;
    typedef int TrackId;
    typedef float Purity;
    typedef float Efficiency;
  }

  // Templated module
  template <typename TrgTrackType>
  class CDCTriggerRecoMatcherModuleT : public Module {
  public:
    CDCTriggerRecoMatcherModuleT() : Module()
    {
      setDescription("A module to match trigger tracks to RecoTracks.\n"
                     "It then makes relations from RecoTracks to trigger tracks.");

      setPropertyFlags(c_ParallelProcessingCertified);

      addParam("RecoTrackCollectionName", m_RecoTrackCollectionName,
               "Name of the RecoTrack StoreArray to be matched.",
               std::string("RecoTracks"));
      addParam("TrgTrackCollectionName", m_TrgTrackCollectionName,
               "Name of the trigger track StoreArray to be matched.",
               std::string("TRGCDC2DFinderTracks"));
      addParam("hitCollectionName", m_hitCollectionName,
               "Name of the StoreArray containing hits used for matching",
               std::string(""));
      addParam("axialOnly", m_axialOnly,
               "Switch for 2D matching (axial only).",
               false);
      addParam("minPurity", m_minPurity,
               "Minimum purity for matching.",
               0.1);
      addParam("minEfficiency", m_minEfficiency,
               "Minimum efficiency for matching.",
               0.1);
      addParam("relateClonesAndMerged", m_relateClonesAndMerged,
               "Switch for creating relations for clones and merged tracks.",
               true);
      addParam("relateHitsByID", m_relateHitsByID,
               "Switch for creating hit relations based on wire ID.",
               true);
    }

    virtual ~CDCTriggerRecoMatcherModuleT() {}

    void initialize() override
    {
      m_segmentHits.isRequired(m_hitCollectionName);
      m_trgTracks.isRequired(m_TrgTrackCollectionName);
      m_recoTracks.isRequired(m_RecoTrackCollectionName);

      m_trgTracks.requireRelationTo(m_segmentHits);
      m_recoTracks.registerRelationTo(m_segmentHits);
      m_recoTracks.registerRelationTo(m_trgTracks);
      m_trgTracks.registerRelationTo(m_recoTracks);
    }

    void event() override
    {
      for (int ireco = 0; ireco < m_recoTracks.getEntries(); ++ireco) {
        RecoTrack* recoTrack = m_recoTracks[ireco];
        // Skip if relations already exist
        if (recoTrack->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName).size() > 0) continue;

        for (CDCHit* cdcHit : recoTrack->getCDCHitList()) {
          if (m_relateHitsByID) {
            for (CDCTriggerSegmentHit& tsHit : m_segmentHits) {
              if (tsHit.getID() == cdcHit->getID()) {
                recoTrack->addRelationTo(&tsHit);
              }
            }
          } else {
            // Look for relations between CDC hits and TS hits
            auto relHits = cdcHit->template getRelationsFrom<CDCTriggerSegmentHit>(m_hitCollectionName);
            for (size_t i = 0; i < relHits.size(); ++i) {
              // Create relations only for priority hits (relation weight 2)
              if (relHits.weight(i) > 1)
                recoTrack->addRelationTo(relHits[i]);
            }
          }
        }
      }

      // Early exit if no tracks are present
      int nRecoTracks = m_recoTracks.getEntries();
      int nTrgTracks = m_trgTracks.getEntries();
      if (nRecoTracks == 0 || nTrgTracks == 0) return;

      // Helper to build multimap of hitId -> trackId
      auto buildHitToTrackMap = [&](auto & tracks) {
        std::multimap<HitId, TrackId> result;
        for (TrackId id = 0; id < tracks.getEntries(); ++id) {
          auto relHits = tracks[id]->template getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
          for (auto& hit : relHits) {
            result.insert(std::make_pair(hit.getArrayIndex(), id));
          }
        }
        return result;
      };

      auto recoTrackIdByHitId = buildHitToTrackMap(m_recoTracks);
      auto trgTrackIdByHitId = buildHitToTrackMap(m_trgTracks);

      Eigen::MatrixXi confusionMatrix = Eigen::MatrixXi::Zero(nTrgTracks, nRecoTracks);
      Eigen::RowVectorXi totalHitsByRecoTrackId = Eigen::RowVectorXi::Zero(nRecoTracks);
      Eigen::VectorXi totalHitsByTrgTrackId = Eigen::VectorXi::Zero(nTrgTracks);

      // Fill the confusion matrix (add a weight if a hit is shared)
      for (HitId hitId = 0; hitId < m_segmentHits.getEntries(); ++hitId) {
        if (m_axialOnly && m_segmentHits[hitId]->getISuperLayer() % 2) continue;

        auto trgRange = trgTrackIdByHitId.equal_range(hitId);
        auto recoRange = recoTrackIdByHitId.equal_range(hitId);

        for (auto& [_, tId] : as_range(trgRange)) totalHitsByTrgTrackId(tId) += 1;
        for (auto& [_, rId] : as_range(recoRange)) totalHitsByRecoTrackId(rId) += 1;

        for (auto& [_, rId] : as_range(recoRange)) {
          for (auto& [_, tId] : as_range(trgRange)) {
            confusionMatrix(tId, rId) += 1;
          }
        }
      }

      // Helper to build map of bestRecoId -> purity
      auto bestRecoForTrg = [&](TrackId trgId) {
        Eigen::RowVectorXi row = confusionMatrix.row(trgId);
        Eigen::RowVectorXi::Index bestRecoId;
        int hits = row.maxCoeff(&bestRecoId);
        Purity purity = Purity(hits) / totalHitsByTrgTrackId(trgId);
        return std::pair<TrackId, Purity>(bestRecoId, purity);
      };

      // Helper to build map of bestTrgId -> eff
      auto bestTrgForReco = [&](TrackId recoId) {
        Eigen::VectorXi col = confusionMatrix.col(recoId);
        Eigen::VectorXi::Index bestTrgId;
        int hits = col.maxCoeff(&bestTrgId);
        Efficiency eff = Efficiency(hits) / totalHitsByRecoTrackId(recoId);
        return std::pair<TrackId, Efficiency>(bestTrgId, eff);
      };

      // Precompute relations
      std::vector<std::pair<TrackId, Purity>> purestRecoByTrg(nTrgTracks);
      for (TrackId t = 0; t < nTrgTracks; ++t) purestRecoByTrg[t] = bestRecoForTrg(t);

      std::vector<std::pair<TrackId, Efficiency>> mostEffTrgByReco(nRecoTracks);
      for (TrackId r = 0; r < nRecoTracks; ++r) mostEffTrgByReco[r] = bestTrgForReco(r);

      // Classification of trigger tracks
      for (TrackId t = 0; t < nTrgTracks; ++t) {
        auto [r, purity] = purestRecoByTrg[t];
        TrgTrackType* trg = m_trgTracks[t];

        if (purity < m_minPurity) continue; // Ghost track

        auto [bestTrg, eff] = mostEffTrgByReco[r];
        RecoTrack* reco = m_recoTracks[r];

        if (t == bestTrg) {
          trg->addRelationTo(reco, purity); // Matched track
        } else if (m_relateClonesAndMerged) {
          trg->addRelationTo(reco, -purity); // Clone track
        }
      }

      // Classification of reco tracks
      for (TrackId r = 0; r < nRecoTracks; ++r) {
        auto [t, eff] = mostEffTrgByReco[r];
        RecoTrack* reco = m_recoTracks[r];

        if (eff < m_minEfficiency) continue; // Missing track

        auto [bestReco, purity] = purestRecoByTrg[t];
        TrgTrackType* trg = m_trgTracks[t];

        if (r == bestReco) {
          reco->addRelationTo(trg, eff); // Matched track
        } else if (m_relateClonesAndMerged) {
          reco->addRelationTo(trg, -eff); // Merged track
        }
      }
    }

  private:
    std::string m_RecoTrackCollectionName;
    std::string m_TrgTrackCollectionName;
    std::string m_hitCollectionName;
    bool m_axialOnly{false};
    double m_minPurity{0.1};
    double m_minEfficiency{0.1};
    bool m_relateClonesAndMerged{true};
    bool m_relateHitsByID{true};

    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    StoreArray<TrgTrackType> m_trgTracks;
    StoreArray<RecoTrack> m_recoTracks;
  };

  // Aliases for the the two modules
  class CDCTriggerRecoMatcherModule : public CDCTriggerRecoMatcherModuleT<CDCTriggerTrack> {
  public:
    CDCTriggerRecoMatcherModule() : CDCTriggerRecoMatcherModuleT<CDCTriggerTrack>() {}
  };

  class CDCTrigger3DHRecoMatcherModule : public CDCTriggerRecoMatcherModuleT<CDCTrigger3DHTrack> {
  public:
    CDCTrigger3DHRecoMatcherModule() : CDCTriggerRecoMatcherModuleT<CDCTrigger3DHTrack>() {}
  };
}
