/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/mcMatcher/MCRecoTracksMatcherModule.h>

// datastore types
#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>

// hit types
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>

#include <map>

#include <Eigen/Dense>

/* disables the false positive warning of Intel icc about iter_pair_range::epmty() being unused
tracking/modules/mcMatcher/src/MCRecoTracksMatcherModule.cc:55: warning #177: function
"::iter_pair_range::empty [with Iter=std::_Rb_tree_const_iterator::DetId={Belle2::Const::EDetector},
::HitId={int}>, ::WeightedRecoTrackId>>]" was declared but never referenced
bool empty() const
*/
#ifdef __INTEL_COMPILER
#pragma warning disable 177
#endif

using namespace Belle2;

REG_MODULE(MCRecoTracksMatcher);

namespace {
  // small anonymous helper construct making converting a pair of iterators usable
  // with range based for
  template <class Iter>
  struct iter_pair_range : std::pair<Iter, Iter> {
    iter_pair_range(std::pair<Iter, Iter> const& x)
      : std::pair<Iter, Iter>(x)
    {
    }

    Iter begin() const
    {
      return this->first;
    }

    Iter end() const
    {
      return this->second;
    }

    bool empty() const
    {
      return begin() == end();
    }
  };

  template <class Iter>
  inline iter_pair_range<Iter> as_range(std::pair<Iter, Iter> const& x)
  {
    return iter_pair_range<Iter>(x);
  }

  using DetId = Const::EDetector;
  using HitId = int;
  using RecoTrackId = int;
  struct WeightedRecoTrackId {
    operator int() const { return id; }
    RecoTrackId id;
    double weight;
  };

  using DetHitIdPair = std::pair<DetId, HitId>;

  struct CompDetHitIdPair {
    bool operator()(const std::pair<DetId, HitId>& lhs,
                    const std::pair<std::pair<DetId, HitId>, WeightedRecoTrackId>& rhs)
    {
      return lhs < rhs.first;
    }

    bool operator()(const std::pair<std::pair<DetId, HitId>, WeightedRecoTrackId>& lhs,
                    const std::pair<DetId, HitId>& rhs)
    {
      return lhs.first < rhs;
    }
  };

  // anonymous helper function to fill a set or a map with the hit IDs and det IDs (we need both a set or a map in the following).
  template <class AMapOrSet>
  void fillIDsFromStoreArray(AMapOrSet& recoTrackID_by_hitID,
                             const StoreArray<RecoTrack>& storedRecoTracks)
  {
    RecoTrackId recoTrackId = -1;
    for (const RecoTrack& recoTrack : storedRecoTracks) {
      ++recoTrackId;
      std::vector<std::pair<DetHitIdPair, WeightedRecoTrackId> > hitIDsInTrack;
      double totalWeight = 0;
      using OriginTrackFinder = RecoHitInformation::OriginTrackFinder;
      const OriginTrackFinder c_MCTrackFinderAuxiliaryHit =
        OriginTrackFinder::c_MCTrackFinderAuxiliaryHit;

      for (const RecoHitInformation::UsedCDCHit* cdcHit : recoTrack.getCDCHitList()) {
        OriginTrackFinder originFinder = recoTrack.getFoundByTrackFinder(cdcHit);
        double weight = originFinder == c_MCTrackFinderAuxiliaryHit ? 0 : 1;
        totalWeight += weight;
        hitIDsInTrack.push_back({{Const::CDC, cdcHit->getArrayIndex()}, {recoTrackId, weight}});
      }
      for (const RecoHitInformation::UsedSVDHit* svdHit : recoTrack.getSVDHitList()) {
        OriginTrackFinder originFinder = recoTrack.getFoundByTrackFinder(svdHit);
        double weight = originFinder == c_MCTrackFinderAuxiliaryHit ? 0 : 1;
        totalWeight += weight;
        hitIDsInTrack.push_back({{Const::SVD, svdHit->getArrayIndex()}, {recoTrackId, weight}});
      }
      for (const RecoHitInformation::UsedPXDHit* pxdHit : recoTrack.getPXDHitList()) {
        OriginTrackFinder originFinder = recoTrack.getFoundByTrackFinder(pxdHit);
        double weight = originFinder == c_MCTrackFinderAuxiliaryHit ? 0 : 1;
        totalWeight += weight;
        hitIDsInTrack.push_back({{Const::PXD, pxdHit->getArrayIndex()}, {recoTrackId, weight}});
      }

      // In case all hits are auxiliary for a track - reset all weights to 1
      if (totalWeight == 0) {
        for (std::pair<DetHitIdPair, WeightedRecoTrackId>& recoTrack_for_hitID : hitIDsInTrack) {
          recoTrack_for_hitID.second.weight = 1;
        }
      }

      // Commit to output
      typename AMapOrSet::iterator itInsertHint = recoTrackID_by_hitID.end();
      for (std::pair<DetHitIdPair, WeightedRecoTrackId>& recoTrack_for_hitID : hitIDsInTrack) {
        itInsertHint = recoTrackID_by_hitID.insert(itInsertHint, recoTrack_for_hitID);
      }
    }
  }
}

MCRecoTracksMatcherModule::MCRecoTracksMatcherModule()
  : Module()
{
  setDescription("This module compares reconstructed tracks generated by some pattern recognition "
                 "algorithm for PXD, SVD and/or CDC to ideal Monte Carlo tracks and performs a "
                 "matching from the former to the underlying MCParticles.");

  setPropertyFlags(c_ParallelProcessingCertified);

  //Parameter definition
  // Inputs
  addParam("prRecoTracksStoreArrayName",
           m_param_prRecoTracksStoreArrayName,
           "Name of the collection containing the tracks as generate a patter recognition algorithm to be evaluated ",
           std::string(""));

  addParam("mcRecoTracksStoreArrayName",
           m_param_mcRecoTracksStoreArrayName,
           "Name of the collection containing the reference tracks as generate by a Monte-Carlo-Tracker (e.g. MCTrackFinder)",
           std::string("MCGFTrackCands"));

  // Hit content to be evaluated
  addParam("UsePXDHits",
           m_param_usePXDHits,
           "Set true if PXDHits or PXDClusters should be used in the matching in case they are present",
           true);

  addParam("UseSVDHits",
           m_param_useSVDHits,
           "Set true if SVDHits or SVDClusters should be used in the matching in case they are present",
           true);

  addParam("UseCDCHits",
           m_param_useCDCHits,
           "Set true if CDCHits should be used in the matching in case they are present",
           true);

  addParam("UseOnlyAxialCDCHits",
           m_param_useOnlyAxialCDCHits,
           "Set true if only the axial CDCHits should be used",
           false);

  addParam("MinimalPurity",
           m_param_minimalPurity,
           "Minimal purity of a PRTrack to be considered matchable to a MCTrack. "
           "This number encodes how many correct hits are minimally need to compensate for a false hits. "
           "The default 2.0 / 3.0 suggests that for each background hit can be compensated by two correct hits.",
           2.0 / 3.0);

  addParam("MinimalEfficiency",
           m_param_minimalEfficiency,
           "Minimal efficiency of a MCTrack to be considered matchable to a PRTrack. "
           "This number encodes which fraction of the true hits must at least be in the reconstructed track. "
           "The default 0.05 suggests that at least 5% of the true hits should have been picked up.",
           0.05);
}

void MCRecoTracksMatcherModule::initialize()
{
  // Check if there are MC Particles
  StoreArray<MCParticle> storeMCParticles;

  if (storeMCParticles.isOptional()) {
    m_mcParticlesPresent = true;

    // Actually retrieve the StoreArrays
    StoreArray<RecoTrack> storePRRecoTracks(m_param_prRecoTracksStoreArrayName);
    StoreArray<RecoTrack> storeMCRecoTracks(m_param_mcRecoTracksStoreArrayName);

    // Require both RecoTrack arrays and the MCParticles to be present in the DataStore
    storePRRecoTracks.isRequired();
    storeMCRecoTracks.isRequired();
    storeMCParticles.isRequired();

    // Extract the default names for the case empty stings were given
    m_param_prRecoTracksStoreArrayName = storePRRecoTracks.getName();
    m_param_mcRecoTracksStoreArrayName = storeMCRecoTracks.getName();

    // Purity relation - for each PRTrack to store the purest MCTrack
    storePRRecoTracks.registerRelationTo(storeMCRecoTracks);

    // Efficiency relation - for each MCTrack to store the most efficient PRTrack
    storeMCRecoTracks.registerRelationTo(storePRRecoTracks);

    // MCParticle matching relation - purity
    storePRRecoTracks.registerRelationTo(storeMCParticles);

    // MCParticle matching relation - efficiency
    storeMCParticles.registerRelationTo(storePRRecoTracks);

    // Announce optional store arrays to the hits or clusters in case they should be used
    // We make them optional in case of limited detector setup.
    // PXD
    if (m_param_usePXDHits) {
      StoreArray<PXDCluster> pxdClusters;
      pxdClusters.isOptional();
    }

    // SVD
    if (m_param_useSVDHits) {
      StoreArray<SVDCluster> svdClusters;
      svdClusters.isOptional();
    }

    // CDC
    if (m_param_useCDCHits) {
      StoreArray<CDCHit> cdcHits;
      cdcHits.isOptional();
    }
  }
}

void MCRecoTracksMatcherModule::event()
{
  // Skip in the case there are no MC particles present.
  if (not m_mcParticlesPresent) {
    B2DEBUG(100, "Skipping MC Track Matcher as there are no MC Particles registered in the DataStore.");
    return;
  }

  B2DEBUG(100, "########## MCRecoTracksMatcherModule ############");

  // Fetch store array
  StoreArray<RecoTrack> prRecoTracks(m_param_prRecoTracksStoreArrayName);
  StoreArray<RecoTrack> mcRecoTracks(m_param_mcRecoTracksStoreArrayName);
  StoreArray<MCParticle> mcParticles("");

  int nMCRecoTracks = mcRecoTracks.getEntries();
  int nPRRecoTracks = prRecoTracks.getEntries();

  B2DEBUG(100, "Number patter recognition tracks is " << nPRRecoTracks);
  B2DEBUG(100, "Number Monte-Carlo tracks is " << nMCRecoTracks);

  if (not nMCRecoTracks or not nPRRecoTracks) {
    // Neither no pattern recognition tracks
    // or no Monte Carlo tracks are present in this event
    // Cannot perform matching.
    return;
  }

  // ### Build a detector_id hit_id to reco track map for easier lookup later ###
  std::multimap<DetHitIdPair, WeightedRecoTrackId > mcId_by_hitId;
  fillIDsFromStoreArray(mcId_by_hitId, mcRecoTracks);

  //  Use set instead of multimap to handel to following situation
  //  * One hit may be assigned to multiple tracks should contribute to the efficiency of both tracks
  //  * One hit assigned twice or more to the same track should not contribute to the purity multiple times
  //  The first part is handled well by the multimap. But to enforce that one hit is also only assigned
  //  once to a track we use a set.
  std::set<std::pair<DetHitIdPair, WeightedRecoTrackId>> prId_by_hitId;
  fillIDsFromStoreArray(prId_by_hitId, prRecoTracks);

  // ### Get the number of relevant hits for each detector ###
  // Since we are mostly dealing with indices in this module, this is all we need from the StoreArray
  // Silently skip store arrays that are not present in reduced detector setups.
  std::map<DetId, int> nHits_by_detId;

  // PXD
  if (m_param_usePXDHits) {
    StoreArray<PXDCluster> pxdClusters;
    if (pxdClusters.isOptional()) {
      int nHits = pxdClusters.getEntries();
      nHits_by_detId[Const::PXD] = nHits;
    }
  }

  // SVD
  if (m_param_useSVDHits) {
    StoreArray<SVDCluster> svdClusters;
    if (svdClusters.isOptional()) {
      int nHits = svdClusters.getEntries();
      nHits_by_detId[Const::SVD] = nHits;
    }
  }

  // CDC
  if (m_param_useCDCHits) {
    StoreArray<CDCHit> cdcHits;
    if (cdcHits.isOptional()) {
      nHits_by_detId[Const::CDC] = cdcHits.getEntries();
    }
  }

  //### Build the confusion matrix ###

  // Reserve enough space for the confusion matrix
  // The last row is meant for hits not assigned to a mcRecoTrack (aka background hits)
  Eigen::MatrixXd confusionMatrix = Eigen::MatrixXd::Zero(nPRRecoTracks, nMCRecoTracks + 1);
  Eigen::MatrixXd weightedConfusionMatrix = Eigen::MatrixXd::Zero(nPRRecoTracks, nMCRecoTracks + 1);

  // Accumulated the total number of hits/ndf for each Monte-Carlo track separately to avoid double counting,
  // in case patter recognition tracks share hits.
  Eigen::RowVectorXd totalNDF_by_mcId = Eigen::RowVectorXd::Zero(nMCRecoTracks + 1);
  Eigen::RowVectorXd totalWeight_by_mcId = Eigen::RowVectorXd::Zero(nMCRecoTracks + 1);

  // Accumulated the total number of hits/ndf for each pattern recognition track separately to avoid double counting,
  // in case Monte-Carlo tracks share hits.
  Eigen::VectorXd totalNDF_by_prId = Eigen::VectorXd::Zero(nPRRecoTracks);

  // Column index for the hits not assigned to any MCRecoTrack
  const int mcBkgId = nMCRecoTracks;

  // for each detector examine every hit to which mcRecoTrack and prRecoTrack it belongs
  // if the hit is not part of any mcRecoTrack put the hit in the background column.
  for (const std::pair<const DetId, NDF>& detId_nHits_pair : nHits_by_detId) {

    DetId detId = detId_nHits_pair.first;
    int nHits = detId_nHits_pair.second;
    NDF ndfForOneHit = m_ndf_by_detId[detId];

    for (HitId hitId = 0; hitId < nHits; ++hitId) {
      DetHitIdPair detId_hitId_pair(detId, hitId);

      if (m_param_useOnlyAxialCDCHits and detId == Const::CDC) {
        StoreArray<CDCHit> cdcHits;
        const CDCHit* cdcHit = cdcHits[hitId];
        if (cdcHit->getISuperLayer() % 2 != 0) {
          // Skip stereo hits
          continue;
        }
      }

      // Seek all Monte Carlo tracks with the given hit
      const auto mcIds_for_detId_hitId_pair =
        as_range(mcId_by_hitId.equal_range(detId_hitId_pair));

      // Seek all pattern recognition tracks with the given hit
      const auto prIds_for_detId_hitId_pair =
        as_range(std::equal_range(prId_by_hitId.begin(),
                                  prId_by_hitId.end(),
                                  detId_hitId_pair,
                                  CompDetHitIdPair()));

      // Assign the hits/ndf to the total ndf vector separately to avoid double counting,
      // if patter recognition track share hits.
      if (mcIds_for_detId_hitId_pair.empty()) {
        // If the hit is not assigned to any mcRecoTrack
        // The hit is assigned to the background column
        RecoTrackId mcId = mcBkgId;
        double mcWeight = 1;
        totalNDF_by_mcId(mcId) += ndfForOneHit;
        totalWeight_by_mcId(mcId) += ndfForOneHit * mcWeight;
      } else {
        for (const auto& detId_hitId_pair_and_mcId : mcIds_for_detId_hitId_pair) {
          WeightedRecoTrackId mcId = detId_hitId_pair_and_mcId.second;
          double mcWeight = mcId.weight;
          totalNDF_by_mcId(mcId) += ndfForOneHit;
          totalWeight_by_mcId(mcId) += ndfForOneHit * mcWeight;
        }
      }

      // Assign the hits/ndf to the total ndf vector separately here to avoid double counting,
      // if Monte-Carlo track share hits.
      for (const auto& detId_hitId_pair_and_prId : prIds_for_detId_hitId_pair) {
        RecoTrackId prId = detId_hitId_pair_and_prId.second;
        totalNDF_by_prId(prId) += ndfForOneHit;
      }

      // Fill the confusion matrix
      for (const auto& detId_hitId_pair_and_prId : prIds_for_detId_hitId_pair) {
        RecoTrackId prId = detId_hitId_pair_and_prId.second;
        if (mcIds_for_detId_hitId_pair.empty()) {
          RecoTrackId mcId = mcBkgId;
          double mcWeight = 1;
          confusionMatrix(prId, mcId) += ndfForOneHit;
          weightedConfusionMatrix(prId, mcId) += ndfForOneHit * mcWeight;
        } else {
          for (const auto& detId_hitId_pair_and_mcId : mcIds_for_detId_hitId_pair) {
            WeightedRecoTrackId mcId = detId_hitId_pair_and_mcId.second;
            double mcWeight = mcId.weight;
            confusionMatrix(prId, mcId) += ndfForOneHit;
            weightedConfusionMatrix(prId, mcId) += ndfForOneHit * mcWeight;
          }
        }
      } // end for
    } // end for hitId
  } // end for detId

  B2DEBUG(200, "Confusion matrix of the event : " << std::endl <<  confusionMatrix);
  B2DEBUG(200, "Weighted confusion matrix of the event : " << std::endl <<  weightedConfusionMatrix);

  B2DEBUG(200, "totalNDF_by_mcId : " << std::endl << totalNDF_by_mcId);
  B2DEBUG(200, "totalWeight_by_mcId : " << std::endl << totalWeight_by_mcId);

  B2DEBUG(200, "totalNDF_by_prId : " << std::endl << totalNDF_by_prId);

  Eigen::MatrixXd purityMatrix = confusionMatrix.array().colwise() / totalNDF_by_prId.array();
  Eigen::MatrixXd efficiencyMatrix = confusionMatrix.array().rowwise() / totalNDF_by_mcId.array();
  Eigen::MatrixXd weightedEfficiencyMatrix = weightedConfusionMatrix.array().rowwise() / totalWeight_by_mcId.array();

  B2DEBUG(100, "Purities");
  B2DEBUG(100, purityMatrix);

  B2DEBUG(100, "Efficiencies");
  B2DEBUG(100, efficiencyMatrix);

  B2DEBUG(100, "Weighted efficiencies");
  B2DEBUG(100, weightedEfficiencyMatrix);

  // ### Building the Monte-Carlo track to highest efficiency patter recognition track relation ###
  // Weighted efficiency
  using Efficiency = float;
  using Purity = float;

  struct MostWeightEfficientPRId {
    RecoTrackId id;
    Efficiency weightedEfficiency;
    Efficiency efficiency;
  };
  std::vector<MostWeightEfficientPRId> mostWeightEfficientPRId_by_mcId(nMCRecoTracks);
  for (RecoTrackId mcId = 0; mcId < nMCRecoTracks; ++mcId) {
    Eigen::VectorXd efficiencyCol = efficiencyMatrix.col(mcId);
    Eigen::VectorXd weightedEfficiencyCol = weightedEfficiencyMatrix.col(mcId);

    RecoTrackId bestPrId = 0;
    Efficiency bestWeightedEfficiency = weightedEfficiencyCol(0);
    Efficiency bestEfficiency = efficiencyCol(0);
    Purity bestPurity = purityMatrix.row(0)(mcId);

    // Reject efficiency smaller than the minimal one
    if (bestWeightedEfficiency < m_param_minimalEfficiency) {
      bestWeightedEfficiency = 0;
    }

    // In case of a tie in the weighted efficiency we use the regular efficiency to break it.
    for (RecoTrackId prId = 1; prId < nPRRecoTracks; ++prId) {
      Eigen::RowVectorXd purityRow = purityMatrix.row(prId);

      Efficiency currentWeightedEfficiency = weightedEfficiencyCol(prId);
      Efficiency currentEfficiency = efficiencyCol(prId);
      Purity currentPurity = purityRow(mcId);

      // Reject efficiency smaller than the minimal one
      if (currentWeightedEfficiency < m_param_minimalEfficiency) {
        currentWeightedEfficiency = 0;
      }

      if (std::tie(currentWeightedEfficiency, currentEfficiency, currentPurity) >
          std::tie(bestWeightedEfficiency, bestEfficiency, bestPurity)) {
        bestPrId = prId;
        bestEfficiency = currentEfficiency;
        bestWeightedEfficiency = currentWeightedEfficiency;
        bestPurity = currentPurity;
      }
    }

    bestWeightedEfficiency = weightedEfficiencyCol(bestPrId);
    bestEfficiency = efficiencyCol(bestPrId);
    mostWeightEfficientPRId_by_mcId[mcId] = {bestPrId, bestWeightedEfficiency, bestEfficiency};
  }

  // ### Building the patter recognition track to highest purity Monte-Carlo track relation ###
  // Unweighted purity
  struct MostPureMCId {
    RecoTrackId id;
    Purity purity;
  };

  std::vector<MostPureMCId> mostPureMCId_by_prId(nPRRecoTracks);
  for (int prId = 0; prId < nPRRecoTracks; ++prId) {
    Eigen::RowVectorXd purityRow = purityMatrix.row(prId);

    int mcId;
    Purity highestPurity = purityRow.maxCoeff(&mcId);

    mostPureMCId_by_prId[prId] = {mcId, highestPurity};
  }

  // Log the  Monte-Carlo track to highest efficiency patter recognition track relation
  // Weighted efficiency
  {
    RecoTrackId mcId = -1;
    B2DEBUG(200, "MCTrack to highest weighted efficiency PRTrack relation");
    for (const auto& mostWeightEfficientPRId_for_mcId : mostWeightEfficientPRId_by_mcId) {
      ++mcId;
      const Efficiency& weightedEfficiency = mostWeightEfficientPRId_for_mcId.weightedEfficiency;
      const RecoTrackId& prId = mostWeightEfficientPRId_for_mcId.id;
      B2DEBUG(200,
              "mcId : " << mcId << " ->  prId : " << prId << " with weighted efficiency "
              << weightedEfficiency);
    }
  }

  // Log the patter recognition track to highest purity Monte-Carlo track relation
  // Unweighted purity
  {
    RecoTrackId prId = -1;
    B2DEBUG(200, "PRTrack to highest purity MCTrack relation");
    for (const auto& mostPureMCId_for_prId : mostPureMCId_by_prId) {
      ++prId;
      const RecoTrackId& mcId = mostPureMCId_for_prId.id;
      const Purity& purity = mostPureMCId_for_prId.purity;
      B2DEBUG(200, "prId : " << prId << " ->  mcId : " << mcId << " with purity " << purity);
    }
  }

  // Count the categories
  int nMatched{}, nBackground{}, nClones{}, nGhost{};

  // ### Classify the patter recognition tracks ###
  // Means saving the highest purity relation to the data store
  // + setup the PRTrack to MCParticle relation
  // + save the set the MatchingStatus
  for (RecoTrackId prId = 0; prId < nPRRecoTracks; ++prId) {
    RecoTrack* prRecoTrack = prRecoTracks[prId];

    const MostPureMCId& mostPureMCId = mostPureMCId_by_prId[prId];

    const RecoTrackId& mcId = mostPureMCId.id;
    const Purity& purity = mostPureMCId.purity;

    // GHOST
    if (not(purity > 0) or not(purity >= m_param_minimalPurity)) {
      prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_ghost);

      B2DEBUG(100, "Stored PRTrack " << prId << " as ghost because of too low purity");
      continue;
    }

    // BACKGROUND
    if (mcId == mcBkgId) {
      prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_background);

      B2DEBUG(100, "Stored PRTrack " << prId << " as background because of too low purity.");
      ++nBackground;
      continue;
    }

    // After the classification for bad purity and background we examine,
    // whether the highest purity Monte-Carlo track has in turn the highest efficiency
    // patter recognition track equal to this track.
    // All extra patter recognition tracks are marked as clones.

    RecoTrack* mcRecoTrack = mcRecoTracks[mcId];
    MCParticle* mcParticle = mcRecoTrack->getRelated<MCParticle>();
    B2ASSERT("No relation from MCRecoTrack to MCParticle.", mcParticle);

    const MostWeightEfficientPRId& mostWeightEfficientPRId_for_mcId =
      mostWeightEfficientPRId_by_mcId[mcId];

    const RecoTrackId& mostWeightEfficientPRId = mostWeightEfficientPRId_for_mcId.id;
    const Efficiency& weightedEfficiency = mostWeightEfficientPRId_for_mcId.weightedEfficiency;
    // const Efficiency& efficiency = mostWeightEfficientPRId_for_mcId.efficiency;

    // Note : The matched category may also contain higher order clones recognisable by their low
    // absolute efficiency

    // MATCHED
    if (prId == mostWeightEfficientPRId) {
      // Setup the relation purity relation
      prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_matched);
      prRecoTrack->addRelationTo(mcRecoTrack, purity);

      // Add the mc matching relation
      prRecoTrack->addRelationTo(mcParticle, purity);

      B2DEBUG(100, "Stored PRTrack " << prId << " as matched.");
      B2DEBUG(100, "MC Match prId " << prId << " to mcPartId " << mcParticle->getArrayIndex());
      B2DEBUG(100, "Purity rel: prId " << prId << " -> mcId " << mcId << " : " << purity);
      ++nMatched;
      continue;
    }

    // GHOST
    // Pattern recognition track fails the minimal efficiency requirement to be matched.
    // We might want to introduce a different classification here, if we see problems
    // with too many ghosts and want to investigate the specific source of the mismatch.
    if (not(weightedEfficiency >= m_param_minimalEfficiency)) {
      prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_ghost);
      B2DEBUG(100, "Stored PRTrack " << prId << " as ghost because of too low efficiency.");
      ++nGhost;
      continue;
    }

    // Final category
    // CLONE
    // Setup the relation purity relation
    prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_clone);
    prRecoTrack->addRelationTo(mcRecoTrack, -purity);
    prRecoTrack->addRelationTo(mcParticle, -purity);
    ++nClones;

    // Add the mc matching relation to the mc particle
    B2DEBUG(100, "Stored PRTrack " << prId << " as clone.");
    B2DEBUG(100, "MC Match prId " << prId << " to mcPartId " << mcParticle->getArrayIndex());
    B2DEBUG(100, "Purity rel: prId " << prId << " -> mcId " << mcId << " : " << -purity);
  } // end for prId


  B2DEBUG(100, "Number of matches " << nMatched);
  B2DEBUG(100, "Number of clones " << nClones);
  B2DEBUG(100, "Number of bkg " << nBackground);
  B2DEBUG(100, "Number of ghost " << nGhost);

  // ### Classify the Monte-Carlo tracks ###
  // Meaning save the highest weighted efficiency relation to the data store.
  for (RecoTrackId mcId = 0; mcId < nMCRecoTracks; ++mcId) {
    RecoTrack* mcRecoTrack = mcRecoTracks[mcId];
    MCParticle* mcParticle = mcRecoTrack->getRelated<MCParticle>();

    const MostWeightEfficientPRId& mostWeightEfficiencyPRId = mostWeightEfficientPRId_by_mcId[mcId];

    const RecoTrackId& prId = mostWeightEfficiencyPRId.id;
    const Efficiency& weightedEfficiency = mostWeightEfficiencyPRId.weightedEfficiency;
    // const Efficiency& efficiency = mostWeightEfficiencyPRId.efficiency;

    B2ASSERT("Index of pattern recognition tracks out of range.", prId < nPRRecoTracks and prId >= 0);

    RecoTrack* prRecoTrack = prRecoTracks[prId];

    const MostPureMCId& mostPureMCId_for_prId = mostPureMCId_by_prId[prId];
    const RecoTrackId& mostPureMCId = mostPureMCId_for_prId.id;

    // MATCHED
    if (mcId == mostPureMCId and
        (prRecoTrack->getMatchingStatus() == RecoTrack::MatchingStatus::c_matched or
         prRecoTrack->getMatchingStatus() == RecoTrack::MatchingStatus::c_clone)) {
      // Setup the relation with positive weighted efficiency for this case.
      mcRecoTrack->addRelationTo(prRecoTrack, weightedEfficiency);
      mcParticle->addRelationTo(prRecoTrack, weightedEfficiency);
      B2DEBUG(100, "Efficiency rel: mcId " << mcId  << " -> prId " << prId << " : " << weightedEfficiency);
      continue;
    }

    // MERGED
    // This MCTrack has a significant portion of hits in a PRTrack
    // which in turn better describes a MCTrack different form this.
    // Setup the relation with negative weighted efficiency for this case.
    bool isMergedMCRecoTrack =
      weightedEfficiency >= m_param_minimalEfficiency and
      (prRecoTrack->getMatchingStatus() == RecoTrack::MatchingStatus::c_matched or
       prRecoTrack->getMatchingStatus() == RecoTrack::MatchingStatus::c_clone);

    if (isMergedMCRecoTrack) {
      mcRecoTrack->addRelationTo(prRecoTrack, -weightedEfficiency);
      mcParticle->addRelationTo(prRecoTrack, -weightedEfficiency);
      B2DEBUG(100, "Efficiency rel: mcId " << mcId << " -> prId " << prId << " : " << -weightedEfficiency);
      continue;
    }

    // MISSING
    // No related pattern recognition track
    // Do not create a relation.
    B2DEBUG(100, "mcId " << mcId << " is missing. No relation created.");
    B2DEBUG(100, "is Primary" << mcRecoTracks[mcId]->getRelatedTo<MCParticle>()->isPrimaryParticle());
    B2DEBUG(100, "best prId " << prId << " with purity " << mostPureMCId_for_prId.purity << " -> " << mostPureMCId);
    B2DEBUG(100, "MC Total ndf " << totalNDF_by_mcId[mcId]);
    B2DEBUG(100, "MC Total weight" << totalWeight_by_mcId[mcId]);
    B2DEBUG(100, "MC Overlap ndf\n " << confusionMatrix.col(mcId).transpose());
    B2DEBUG(100, "MC Overlap weight\n " << weightedConfusionMatrix.col(mcId).transpose());
    B2DEBUG(100, "MC Efficiencies for the track\n" << efficiencyMatrix.col(mcId).transpose());
    B2DEBUG(100, "MC Weighted efficiencies for the track\n" << weightedEfficiencyMatrix.col(mcId).transpose());
  } // end for mcId

  B2DEBUG(100, "########## End MCRecoTracksMatcherModule ############");

} //end event()
