#include "trg/cdc/modules/mcmatcher/CDCTriggerRecoMatcherModule.h"

#include <cdc/dataobjects/CDCHit.h>

#include <framework/gearbox/Const.h>

#include <Eigen/Dense>

namespace {
  //small anonymous helper construct making converting a pair of iterators usable
  //with range based for
  template<class Iter>
  struct iter_pair_range : std::pair<Iter, Iter> {
    iter_pair_range(std::pair<Iter, Iter> const& x) : std::pair<Iter, Iter>(x) {}
    Iter begin() const {return this->first;}
    Iter end()   const {return this->second;}
  };

  template<class Iter>
  inline iter_pair_range<Iter> as_range(std::pair<Iter, Iter> const& x)
  {
    return iter_pair_range<Iter>(x);
  }

  typedef int HitId;
  typedef int TrackId;
  typedef float Purity;
  typedef float Efficiency;
}



using namespace Belle2;
using namespace std;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerRecoMatcher)

CDCTriggerRecoMatcherModule::CDCTriggerRecoMatcherModule() : Module()
{
  setDescription("A module to match CDCTriggerTracks to RecoTracks.\n"
                 "First makes relations from RecoTracks to CDCTriggerSegmentHits, "
                 "then makes relations from RecoTracks to CDCTriggerTracks "
                 "and vice-versa.");

  addParam("RecoTrackCollectionName", m_RecoTrackCollectionName,
           "Name of the RecoTrack StoreArray to be matched.",
           string("RecoTracks"));
  addParam("TrgTrackCollectionName", m_TrgTrackCollectionName,
           "Name of the CDCTriggerTrack StoreArray to be matched.",
           string("TRGCDC2DFinderTracks"));
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the StoreArray of CDCTriggerSegmentHits used for the matching.",
           string(""));
  addParam("axialOnly", m_axialOnly,
           "Switch to ignore stereo hits (= 2D matching).",
           false);
  addParam("minPurity", m_minPurity,
           "Minimum purity for reconstructed tracks.",
           0.1);
  addParam("minEfficiency", m_minEfficiency,
           "Minimum efficiency for MC tracks.",
           0.1);
  addParam("relateClonesAndMerged", m_relateClonesAndMerged,
           "If true, create relations for clones and merged tracks "
           "(will get negative weight).",
           true);
  addParam("relateHitsByID", m_relateHitsByID,
           "If true, compare ID of CDCTriggerSegmentHits and CDCHits "
           "to create relations, otherwise use only existing relations.",
           true);
}


void
CDCTriggerRecoMatcherModule::initialize()
{
  m_segmentHits.isRequired(m_hitCollectionName);
  m_trgTracks.isRequired(m_TrgTrackCollectionName);
  m_recoTracks.isRequired(m_RecoTrackCollectionName);

  m_trgTracks.requireRelationTo(m_segmentHits);
  m_recoTracks.registerRelationTo(m_segmentHits);
  m_recoTracks.registerRelationTo(m_trgTracks);
  m_trgTracks.registerRelationTo(m_recoTracks);
}


void
CDCTriggerRecoMatcherModule::event()
{
  // create relations from RecoTracks to SegmentHits via CDCHits
  for (int ireco = 0; ireco < m_recoTracks.getEntries(); ++ireco) {
    RecoTrack* recoTrack = m_recoTracks[ireco];
    // if relations exist already, skip this step
    // (matching may be done several times with different trigger tracks)
    if (recoTrack->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName).size() > 0)
      continue;
    vector<CDCHit*> cdcHits = recoTrack->getCDCHitList();
    for (unsigned iHit = 0; iHit < cdcHits.size(); ++iHit) {
      if (m_relateHitsByID) {
        // loop over TS hits and compare ID
        for (CDCTriggerSegmentHit& tsHit : m_segmentHits) {
          if (tsHit.getID() == cdcHits[iHit]->getID()) {
            recoTrack->addRelationTo(&tsHit);
          }
        }
      } else {
        // look for relations between CDC hits and TS hits
        RelationVector<CDCTriggerSegmentHit> relHits =
          cdcHits[iHit]->getRelationsFrom<CDCTriggerSegmentHit>(m_hitCollectionName);
        for (unsigned iTS = 0; iTS < relHits.size(); ++iTS) {
          // create relations only for priority hits (relation weight 2)
          if (relHits.weight(iTS) > 1)
            recoTrack->addRelationTo(relHits[iTS]);
        }
      }
    }
  }

  // derive relations between trigger tracks and reco tracks,
  // following the basic ideas of the MCMatcherTracksModule (tracking)

  B2DEBUG(100, "########## start matching ############");

  int nRecoTracks = m_recoTracks.getEntries();
  int nTrgTracks = m_trgTracks.getEntries();

  B2DEBUG(100, "Number of trigger tracks is " << nTrgTracks);
  B2DEBUG(100, "Number of reco tracks is " << nRecoTracks);

  if (not nRecoTracks or not nTrgTracks) {
    // Either no trigger tracks
    // or no reco tracks are present in this event
    // Cannot perform matching.
    return;
  }

  //### Build a hit_id to track map for easier lookup later ###
  std::multimap<HitId, TrackId> recoTrackId_by_hitId;
  {
    std::multimap<HitId, TrackId>::iterator itRecoInsertHit = recoTrackId_by_hitId.end();
    TrackId recoTrackId = -1;
    for (const RecoTrack& recoTrack : m_recoTracks) {
      ++recoTrackId;
      RelationVector<CDCTriggerSegmentHit> relHits =
        recoTrack.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      for (unsigned iHit = 0; iHit < relHits.size(); ++iHit) {
        const HitId hitId = relHits[iHit]->getArrayIndex();
        itRecoInsertHit = recoTrackId_by_hitId.insert(itRecoInsertHit,
                                                      make_pair(hitId, recoTrackId));
        B2DEBUG(250, "hitId " << hitId << " in SL " << relHits[iHit]->getISuperLayer()
                << ", recoTrackId " << recoTrackId);
      }
    }
  }

  //### Build a hit_id to track map for easier lookup later ###
  std::multimap<HitId, TrackId> trgTrackId_by_hitId;
  {
    std::multimap<HitId, TrackId>::iterator itTrgInsertHit = trgTrackId_by_hitId.end();
    TrackId trgTrackId = -1;
    for (const CDCTriggerTrack& trgTrack : m_trgTracks) {
      ++trgTrackId;
      RelationVector<CDCTriggerSegmentHit> relHits =
        trgTrack.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      for (unsigned int iHit = 0; iHit < relHits.size(); ++iHit) {
        const HitId hitId = relHits[iHit]->getArrayIndex();
        itTrgInsertHit = trgTrackId_by_hitId.insert(itTrgInsertHit,
                                                    make_pair(hitId, trgTrackId));
        B2DEBUG(250, "hitId " << hitId << " in SL " << relHits[iHit]->getISuperLayer()
                << ", trgTrackId " << trgTrackId);
      }
    }
  }

  //### Build the confusion matrix ###

  // Reserve enough space for the confusion matrix
  // In contrast to MC matching, background hits are not counted separately
  Eigen::MatrixXi confusionMatrix = Eigen::MatrixXi::Zero(nTrgTracks, nRecoTracks);

  // Count total number of hits for each track separately
  // to avoid double counting (in case tracks share hits)
  Eigen::RowVectorXi totalHits_by_recoTrackId = Eigen::RowVectorXi::Zero(nRecoTracks);
  Eigen::VectorXi totalHits_by_trgTrackId = Eigen::VectorXi::Zero(nTrgTracks);

  // examine every hit to which recoTrack and trgTrack it belongs.
  for (HitId hitId = 0; hitId < m_segmentHits.getEntries(); ++hitId) {
    // skip stereo hits
    if (m_axialOnly && m_segmentHits[hitId]->getISuperLayer() % 2) continue;

    // Seek all recoTracks and trgTracks
    auto range_trgTrackIds = trgTrackId_by_hitId.equal_range(hitId);
    auto range_recoTrackIds = recoTrackId_by_hitId.equal_range(hitId);

    // Assign the hits to the total vector
    for (const pair<HitId, TrackId>& hitId_and_trgTrackId :
         as_range(range_trgTrackIds)) {
      TrackId trgTrackId = hitId_and_trgTrackId.second;
      totalHits_by_trgTrackId(trgTrackId) += 1;
      B2DEBUG(200, " trgTrackId for total count: " << trgTrackId);
    }
    for (const pair<HitId, TrackId>& hitId_and_recoTrackId :
         as_range(range_recoTrackIds)) {
      TrackId recoTrackId = hitId_and_recoTrackId.second;
      totalHits_by_recoTrackId(recoTrackId) += 1;
      B2DEBUG(200, " recoTrackId for total count: " << recoTrackId);
    }

    // Count matrix entries for all combinations
    for (const pair<HitId, TrackId>& hitId_and_recoTrackId :
         as_range(range_recoTrackIds)) {
      for (const pair<HitId, TrackId>& hitId_and_trgTrackId :
           as_range(range_trgTrackIds)) {
        TrackId recoTrackId = hitId_and_recoTrackId.second;
        TrackId trgTrackId = hitId_and_trgTrackId.second;
        confusionMatrix(trgTrackId, recoTrackId) += 1;
        B2DEBUG(200, " trgTrackId : " <<  trgTrackId  << ";  recoTrackId : " <<  recoTrackId);
      }
    }
  } //end for hitId

  B2DEBUG(200, "Confusion matrix of the event : " << endl <<  confusionMatrix);

  B2DEBUG(200, "totalHits_by_trgTrackId : " << endl << totalHits_by_trgTrackId);
  B2DEBUG(200, "totalHits_by_recoTrackId : " << endl << totalHits_by_recoTrackId);

  // ### Building the trg track to highest purity reco track relation ###
  vector<pair<TrackId, Purity>> purestRecoTrackId_by_trgTrackId(nTrgTracks);

  for (TrackId trgTrackId = 0; trgTrackId < nTrgTracks; ++trgTrackId) {
    Eigen::RowVectorXi trgTrackRow = confusionMatrix.row(trgTrackId);
    Eigen::RowVectorXi::Index purestRecoTrackId;

    //Also sets the index of the highest entry in the row vector
    int highestHits = trgTrackRow.maxCoeff(&purestRecoTrackId);
    int totalHits = totalHits_by_trgTrackId(trgTrackId);

    Purity highestPurity = Purity(highestHits) / totalHits;

    purestRecoTrackId_by_trgTrackId[trgTrackId] =
      pair<TrackId, Purity>(purestRecoTrackId, highestPurity);
  }

  // Log the trg track to highest purity reco track
  // relation to debug output
  {
    TrackId trgTrackId = -1;
    B2DEBUG(200, "TrgTrack to highest purity RecoTrack relation");
    for (const pair< TrackId, Purity>& purestRecoTrackId :
         purestRecoTrackId_by_trgTrackId) {
      ++trgTrackId;
      const Purity& purity = purestRecoTrackId.second;
      const TrackId& recoTrackId = purestRecoTrackId.first;
      B2DEBUG(200, "trgTrackId : " << trgTrackId << " ->  recoTrackId : " << recoTrackId
              << " with purity " << purity);
    }
  }

  // ### Building the reco track to highest efficiency trg track relation ###
  vector<pair<TrackId,  Efficiency>> mostEfficientTrgTrackId_by_recoTrackId(nRecoTracks);

  for (TrackId recoTrackId = 0; recoTrackId < nRecoTracks; ++recoTrackId) {
    Eigen::VectorXi recoTrackCol = confusionMatrix.col(recoTrackId);
    Eigen::VectorXi::Index highestHitsTrgTrackId;

    //Also sets the index of the highest entry in the column vector
    int highestHits = recoTrackCol.maxCoeff(&highestHitsTrgTrackId);
    int totalHits = totalHits_by_recoTrackId(recoTrackId);

    Efficiency highestEfficiency = Efficiency(highestHits) / totalHits;

    mostEfficientTrgTrackId_by_recoTrackId[recoTrackId] =
      pair<TrackId, Efficiency> (highestHitsTrgTrackId, highestEfficiency);
  }

  // Log the  reco track to highest efficiency trg track
  // relation to debug output
  {
    TrackId recoTrackId = -1;
    B2DEBUG(200, "RecoTrack to highest efficiency TrgTrack relation");
    for (const pair<TrackId, Efficiency>& mostEfficientTrgTrackId :
         mostEfficientTrgTrackId_by_recoTrackId) {
      ++recoTrackId;
      const Efficiency& efficiency = mostEfficientTrgTrackId.second;
      const TrackId& trgTrackId = mostEfficientTrgTrackId.first;
      B2DEBUG(200, "recoTrackId : " << recoTrackId << " ->  trgTrackId : " << trgTrackId
              << " with efficiency " << efficiency);
    }
  }

  // ### Classify the trg tracks ###
  for (TrackId trgTrackId = 0; trgTrackId < nTrgTracks; ++trgTrackId) {
    CDCTriggerTrack* trgTrack = m_trgTracks[trgTrackId];

    const pair<TrackId, Purity>& purestRecoTrackId = purestRecoTrackId_by_trgTrackId[trgTrackId];
    const TrackId& recoTrackId = purestRecoTrackId.first;
    const Purity& purity = purestRecoTrackId.second;

    if (!(purity >= m_minPurity)) {
      // GHOST
      B2DEBUG(100, "Classified TrgTrack " << trgTrackId << " as ghost because of too low purity.");
    } else {
      // check whether the highest purity reco track has in turn
      // the highest efficiency trg track equal to this track.
      RecoTrack* recoTrack = m_recoTracks[recoTrackId];

      const pair<TrackId, Efficiency>& mostEfficientTrgTrackId =
        mostEfficientTrgTrackId_by_recoTrackId[recoTrackId];

      const TrackId& trgTrackIdCompare = mostEfficientTrgTrackId.first;

      if (trgTrackId != trgTrackIdCompare) {
        // CLONE
        if (m_relateClonesAndMerged) {
          // Add the reco track matching relation with negative weight
          trgTrack->addRelationTo(recoTrack, -purity);
        }
        B2DEBUG(100, "Classified TrgTrack " << trgTrackId << " as clone -> recoTrackId "
                << recoTrackId << " : " << -purity);
      } else {
        // MATCHED
        //Add the reco track matching relation
        trgTrack->addRelationTo(recoTrack, purity);
        B2DEBUG(100, "Classified TrgTrack " << trgTrackId << " as match -> recoTrackId "
                << recoTrackId << " : " << purity);
      }
    }
  }

  // ### Classify the reco tracks ###
  for (TrackId recoTrackId = 0; recoTrackId < nRecoTracks; ++recoTrackId) {
    RecoTrack* recoTrack = m_recoTracks[recoTrackId];

    const pair<TrackId, Efficiency>& mostEfficiencyTrgTrackId =
      mostEfficientTrgTrackId_by_recoTrackId[recoTrackId];
    const TrackId& trgTrackId = mostEfficiencyTrgTrackId.first;
    const Efficiency& efficiency = mostEfficiencyTrgTrackId.second;

    if (!(efficiency >= m_minEfficiency)) {
      // MISSING
      B2DEBUG(100, "Classified RecoTrack " << recoTrackId << " as missing.");
    } else {
      // check whether the highest efficiency trg track has in turn
      // the highest purity reco track equal to this track.
      CDCTriggerTrack* trgTrack = m_trgTracks[trgTrackId];

      const pair<TrackId, Purity>& purestRecoTrackId =
        purestRecoTrackId_by_trgTrackId[trgTrackId];
      const TrackId& recoTrackIdCompare = purestRecoTrackId.first;

      if (recoTrackId != recoTrackIdCompare) {
        // MERGED
        if (m_relateClonesAndMerged) {
          // Add the trg matching relation with negative weight
          recoTrack->addRelationTo(trgTrack, -efficiency);
        }
        B2DEBUG(100, "Classifid RecoTrack " << recoTrackId << " as merge -> trgTrackId "
                << trgTrackId << " : " << -efficiency);

      } else {
        // MATCHED
        // Add the trg matching relation
        recoTrack->addRelationTo(trgTrack, efficiency);
        B2DEBUG(100, "Classified RecoTrack " << recoTrackId << " as match -> trgTrackId "
                << trgTrackId << " : " << efficiency);
      }
    }
  }

  B2DEBUG(100, "########## end matching ############");
}
