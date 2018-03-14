#include "trg/cdc/modules/mcmatcher/CDCTriggerMCMatcherModule.h"

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
REG_MODULE(CDCTriggerMCMatcher)

CDCTriggerMCMatcherModule::CDCTriggerMCMatcherModule() : Module()
{
  setDescription("A module to match CDCTriggerTracks to MCParticles.\n"
                 "Creates an array MCTracks of trackable MCParticles "
                 "and makes relations from MCTracks to CDCTriggerTracks "
                 "and vice-versa.");

  addParam("MCParticleCollectionName", m_MCParticleCollectionName,
           "Name of the MCParticle StoreArray to be matched.",
           string("MCParticles"));
  addParam("TrgTrackCollectionName", m_TrgTrackCollectionName,
           "Name of the CDCTriggerTrack StoreArray to be matched.",
           string("TRGCDC2DFinderTracks"));
  addParam("MCTrackableCollectionName", m_MCTrackableCollectionName,
           "Name of a new StoreArray holding MCParticles considered as trackable.",
           string("MCTracks"));
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the StoreArray of CDCTriggerSegmentHits used for the matching.",
           string(""));
  addParam("minAxial", m_minAxial,
           "Minimum number of axial hits in different super layers.",
           0);
  addParam("minStereo", m_minStereo,
           "Minimum number of stereo hits in different super layers.",
           0);
  addParam("axialOnly", m_axialOnly,
           "Switch to ignore stereo hits (= 2D matching).",
           false);
  addParam("minPurity", m_minPurity,
           "Minimum purity for reconstructed tracks.",
           0.1);
  addParam("minEfficiency", m_minEfficiency,
           "Minimum efficiency for MC tracks.",
           0.1);
  addParam("onlyPrimaries", m_onlyPrimaries,
           "If true, MCTracks are only made from primary particles.",
           false);
  addParam("relateClonesAndMerged", m_relateClonesAndMerged,
           "If true, create relations for clones and merged tracks "
           "(will get negative weight).",
           true);
}


void
CDCTriggerMCMatcherModule::initialize()
{
  m_segmentHits.isRequired(m_hitCollectionName);
  m_prTracks.isRequired(m_TrgTrackCollectionName);
  m_mcParticles.isRequired(m_MCParticleCollectionName);
  m_mcTracks.registerInDataStore(m_MCTrackableCollectionName);

  m_mcParticles.requireRelationTo(m_segmentHits);
  m_prTracks.requireRelationTo(m_segmentHits);
  m_mcParticles.registerRelationTo(m_mcTracks);
  m_mcParticles.registerRelationTo(m_prTracks);
  m_mcTracks.registerRelationTo(m_segmentHits);
  m_mcTracks.registerRelationTo(m_prTracks);
  m_prTracks.registerRelationTo(m_mcParticles);
  m_prTracks.registerRelationTo(m_mcTracks);
}


void
CDCTriggerMCMatcherModule::event()
{
  // get all trackable particles
  for (int imc = 0; imc < m_mcParticles.getEntries(); ++imc) {
    MCParticle* mcParticle = m_mcParticles[imc];

    // minimum requirement: charged and seen in CDC
    if (!mcParticle->hasSeenInDetector(Const::CDC) || mcParticle->getCharge() == 0)
      continue;

    // reject secondaries
    if (m_onlyPrimaries && !mcParticle->hasStatus(MCParticle::c_PrimaryParticle))
      continue;

    // count super layers with related hits
    int nAxial = 0;
    int nStereo = 0;
    vector<bool> SLcounted;
    SLcounted.assign(9, false);

    RelationVector<CDCTriggerSegmentHit> relHits =
      mcParticle->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
    for (unsigned ihit = 0; ihit < relHits.size(); ++ihit) {
      unsigned iSL = relHits[ihit]->getISuperLayer();
      if (SLcounted[iSL]) continue;
      if (iSL % 2) ++nStereo;
      else ++nAxial;
      SLcounted[iSL] = true;
    }

    if (nAxial < m_minAxial || nStereo < m_minStereo) continue;

    // copy particle to list of trackable particles
    MCParticle* mcTrack = m_mcTracks.appendNew(m_mcTracks.getPtr(), *mcParticle);
    mcParticle->addRelationTo(mcTrack);
    for (unsigned ihit = 0; ihit < relHits.size(); ++ihit) {
      mcTrack->addRelationTo(relHits[ihit]);
    }
  }

  // derive relations between trigger tracks and MC tracks,
  // following the basic ideas of the MCMatcherTracksModule (tracking)

  B2DEBUG(100, "########## start MC matching ############");

  int nMCTracks = m_mcTracks.getEntries();
  int nPRTracks = m_prTracks.getEntries();

  B2DEBUG(100, "Number of pattern recognition tracks is " << nPRTracks);
  B2DEBUG(100, "Number of Monte-Carlo tracks is " << nMCTracks);

  if (not nMCTracks or not nPRTracks) {
    // Either no pattern recognition tracks
    // or no Monte Carlo tracks are present in this event
    // Cannot perform matching.
    return;
  }

  //### Build a hit_id to track map for easier lookup later ###
  std::map<HitId, TrackId> mcTrackId_by_hitId;
  {
    std::map<HitId, TrackId>::iterator itMCInsertHit = mcTrackId_by_hitId.end();
    TrackId mcTrackId = -1;
    for (const MCParticle& mcTrack : m_mcTracks) {
      ++mcTrackId;
      RelationVector<CDCTriggerSegmentHit> relHits =
        mcTrack.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      for (unsigned iHit = 0; iHit < relHits.size(); ++iHit) {
        const HitId hitId = relHits[iHit]->getArrayIndex();
        itMCInsertHit = mcTrackId_by_hitId.insert(itMCInsertHit,
                                                  make_pair(hitId, mcTrackId));
        B2DEBUG(250, "hitId " << hitId << " in SL " << relHits[iHit]->getISuperLayer()
                << ", mcTrackId " << mcTrackId);
      }
    }
  }

  //### Build a hit_id to track map for easier lookup later ###
  std::multimap<HitId, TrackId> prTrackId_by_hitId;
  {
    std::multimap<HitId, TrackId>::iterator itPRInsertHit = prTrackId_by_hitId.end();
    TrackId prTrackId = -1;
    for (const CDCTriggerTrack& prTrack : m_prTracks) {
      ++prTrackId;
      RelationVector<CDCTriggerSegmentHit> relHits =
        prTrack.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      for (unsigned int iHit = 0; iHit < relHits.size(); ++iHit) {
        const HitId hitId = relHits[iHit]->getArrayIndex();
        itPRInsertHit = prTrackId_by_hitId.insert(itPRInsertHit,
                                                  make_pair(hitId, prTrackId));
        B2DEBUG(250, "hitId " << hitId << " in SL " << relHits[iHit]->getISuperLayer()
                << ", prTrackId " << prTrackId);
      }
    }
  }

  //### Build the confusion matrix ###

  // Reserve enough space for the confusion matrix
  // The last column is meant for hits not assigned to a mcTrack (aka background hits)
  Eigen::MatrixXi confusionMatrix = Eigen::MatrixXi::Zero(nPRTracks, nMCTracks + 1);

  // Count total number of hits for each Monte-Carlo track separately
  // to avoid double counting (in case pattern recognition tracks share hits)
  // and to count hits not associated to any pattern recognition track.
  Eigen::RowVectorXi totalHits_by_mcTrackId = Eigen::RowVectorXi::Zero(nMCTracks + 1);

  // Column index for the hits not assigned to any MCTrackCand
  const int mcBkgId = nMCTracks;

  // examine every hit to which mcTrack and prTrack it belongs.
  // if the hit is not part of any mcTrack put the hit in the background column.
  for (HitId hitId = 0; hitId < m_segmentHits.getEntries(); ++hitId) {
    // skip stereo hits
    if (m_axialOnly && m_segmentHits[hitId]->getISuperLayer() % 2) continue;

    // First search the unique mcTrackId for the hit.
    // If the hit is not assigned to any mcTrack the Id is set to the background column.
    auto it_mcTrackId = mcTrackId_by_hitId.find(hitId);
    TrackId mcTrackId =
      (it_mcTrackId == mcTrackId_by_hitId.end()) ? mcBkgId : it_mcTrackId->second;

    // Assign the hits to the total vector.
    totalHits_by_mcTrackId(mcTrackId) += 1;

    // Seek all prTrackCands
    auto range_prTrackIds = prTrackId_by_hitId.equal_range(hitId);

    // count for every prTrack that has this hit
    for (const pair<HitId, TrackId>& hitId_and_prTrackId :
         as_range(range_prTrackIds)) {
      TrackId prTrackId = hitId_and_prTrackId.second;
      B2DEBUG(200, " prTrackId : " <<  prTrackId  << ";  mcTrackId : " <<  mcTrackId);
      confusionMatrix(prTrackId, mcTrackId) += 1;
    } //end for prTrackId
  } //end for hitId

  Eigen::VectorXi totalHits_by_prTrackId = confusionMatrix.rowwise().sum();

  B2DEBUG(200, "Confusion matrix of the event : " << endl <<  confusionMatrix);

  B2DEBUG(200, "totalHits_by_prTrackId : " << endl << totalHits_by_prTrackId);
  B2DEBUG(200, "totalHits_by_mcTrackId : " << endl << totalHits_by_mcTrackId);

  // ### Building the patter recognition track to highest purity Monte-Carlo track relation ###
  vector<pair<TrackId, Purity>> purestMCTrackId_by_prTrackId(nPRTracks);

  for (TrackId prTrackId = 0; prTrackId < nPRTracks; ++prTrackId) {
    Eigen::RowVectorXi prTrackRow = confusionMatrix.row(prTrackId);
    Eigen::RowVectorXi::Index purestMCTrackId;

    //Also sets the index of the highest entry in the row vector
    int highestHits = prTrackRow.maxCoeff(&purestMCTrackId);
    int totalHits = totalHits_by_prTrackId(prTrackId);

    Purity highestPurity = Purity(highestHits) / totalHits;

    purestMCTrackId_by_prTrackId[prTrackId] =
      pair<TrackId, Purity>(purestMCTrackId, highestPurity);
  }

  // Log the pattern recognition track to highest purity Monte-Carlo track
  // relation to debug output
  {
    TrackId prTrackId = -1;
    B2DEBUG(200, "PRTrack to highest purity MCTrack relation");
    for (const pair< TrackId, Purity>& purestMCTrackId :
         purestMCTrackId_by_prTrackId) {
      ++prTrackId;
      const Purity& purity = purestMCTrackId.second;
      const TrackId& mcTrackId = purestMCTrackId.first;
      B2DEBUG(200, "prTrackId : " << prTrackId << " ->  mcTrackId : " << mcTrackId
              << " with purity " << purity);
    }
  }

  // ### Building the Monte-Carlo track to highest efficiency pattern recognition track relation ###
  vector<pair<TrackId,  Efficiency>> mostEfficientPRTrackId_by_mcTrackId(nMCTracks);

  for (TrackId mcTrackId = 0; mcTrackId < nMCTracks; ++mcTrackId) {
    Eigen::VectorXi mcTrackCol = confusionMatrix.col(mcTrackId);
    Eigen::VectorXi::Index highestHitsPRTrackId;

    //Also sets the index of the highest entry in the column vector
    int highestHits = mcTrackCol.maxCoeff(&highestHitsPRTrackId);
    int totalHits = totalHits_by_mcTrackId(mcTrackId);

    Efficiency highestEfficiency = Efficiency(highestHits) / totalHits;

    mostEfficientPRTrackId_by_mcTrackId[mcTrackId] =
      pair<TrackId, Efficiency> (highestHitsPRTrackId, highestEfficiency);
  }

  // Log the  Monte-Carlo track to highest efficiency pattern recognition track
  // relation to debug output
  {
    TrackId mcTrackId = -1;
    B2DEBUG(200, "MCTrack to highest efficiency PRTrack relation");
    for (const pair<TrackId, Efficiency>& mostEfficientPRTrackId :
         mostEfficientPRTrackId_by_mcTrackId) {
      ++mcTrackId;
      const Efficiency& efficiency = mostEfficientPRTrackId.second;
      const TrackId& prTrackId = mostEfficientPRTrackId.first;
      B2DEBUG(200, "mcTrackId : " << mcTrackId << " ->  prTrackId : " << prTrackId
              << " with efficiency " << efficiency);
    }
  }

  // ### Classify the pattern recognition tracks ###
  for (TrackId prTrackId = 0; prTrackId < nPRTracks; ++prTrackId) {
    CDCTriggerTrack* prTrack = m_prTracks[prTrackId];

    const pair<TrackId, Purity>& purestMCTrackId = purestMCTrackId_by_prTrackId[prTrackId];
    const TrackId& mcTrackId = purestMCTrackId.first;
    const Purity& purity = purestMCTrackId.second;

    if (!(purity >= m_minPurity)) {
      // GHOST
      B2DEBUG(100, "Classified PRTrack " << prTrackId << " as ghost because of too low purity.");
    } else if (mcTrackId == mcBkgId) {
      // BACKGROUND
      B2DEBUG(100, "Classified PRTrack " << prTrackId << " as background.");
    } else {
      // check whether the highest purity Monte-Carlo track has in turn
      // the highest efficiency pattern recognition track equal to this track.
      MCParticle* mcTrack = m_mcTracks[mcTrackId];

      const pair<TrackId, Efficiency>& mostEfficientPRTrackId =
        mostEfficientPRTrackId_by_mcTrackId[mcTrackId];

      const TrackId& prTrackIdCompare = mostEfficientPRTrackId.first;
      const Efficiency& efficiency = mostEfficientPRTrackId.second;

      if (prTrackId != prTrackIdCompare) {
        if (efficiency >= m_minEfficiency) {
          // CLONE
          if (m_relateClonesAndMerged) {
            // Add the mc matching relation with negative weight
            prTrack->addRelationTo(mcTrack, -purity);
            prTrack->addRelationTo(mcTrack->getRelatedFrom<MCParticle>(m_MCParticleCollectionName), -purity);
          }
          B2DEBUG(100, "Classified PRTrack " << prTrackId << " as clone -> mcTrackId "
                  << mcTrackId << " : " << -purity);
        } else {
          // GHOST
          B2DEBUG(100, "Classified PRTrack " << prTrackId
                  << " as ghost because of too low efficiency in purest MCTrack "
                  << "(mcTrackId=" << mcTrackId << ").");
        }
      } else {
        // MATCHED
        //Add the mc matching relation
        prTrack->addRelationTo(mcTrack, purity);
        prTrack->addRelationTo(mcTrack->getRelatedFrom<MCParticle>(m_MCParticleCollectionName), purity);
        B2DEBUG(100, "Classified PRTrack " << prTrackId << " as match -> mcTrackId "
                << mcTrackId << " : " << purity);
      }
    }

  }

  // ### Classify the Monte-Carlo tracks ###
  for (TrackId mcTrackId = 0; mcTrackId < nMCTracks; ++mcTrackId) {
    MCParticle* mcTrack = m_mcTracks[mcTrackId];

    const pair<TrackId, Efficiency>& mostEfficiencyPRTrackId =
      mostEfficientPRTrackId_by_mcTrackId[mcTrackId];
    const TrackId& prTrackId = mostEfficiencyPRTrackId.first;
    const Efficiency& efficiency = mostEfficiencyPRTrackId.second;

    if (!(efficiency >= m_minEfficiency)) {
      // MISSING
      B2DEBUG(100, "Classified MCTrack " << mcTrackId << " as missing.");
    } else {
      // check whether the highest efficiency pattern recognition track has in turn
      // the highest purity Monte-Carlo track equal to this track.
      CDCTriggerTrack* prTrack = m_prTracks[prTrackId];

      const pair<TrackId, Purity>& purestMCTrackId = purestMCTrackId_by_prTrackId[prTrackId];
      const TrackId& mcTrackIdCompare = purestMCTrackId.first;

      if (mcTrackId != mcTrackIdCompare) {
        // MERGED
        if (m_relateClonesAndMerged) {
          // Add the pr matching relation with negative weight
          mcTrack->addRelationTo(prTrack, -efficiency);
          mcTrack->getRelatedFrom<MCParticle>(m_MCParticleCollectionName)->addRelationTo(prTrack, -efficiency);
        }
        B2DEBUG(100, "Classifid MCTrack " << mcTrackId << " as merge -> prTrackId "
                << prTrackId << " : " << -efficiency);

      } else {
        // MATCHED
        // Add the pr matching relation
        mcTrack->addRelationTo(prTrack, efficiency);
        mcTrack->getRelatedFrom<MCParticle>(m_MCParticleCollectionName)->addRelationTo(prTrack, efficiency);
        B2DEBUG(100, "Classified MCTrack " << mcTrackId << " as match -> prTrackId "
                << prTrackId << " : " << efficiency);
      }
    }
  }

  B2DEBUG(100, "########## end MC matching ############");
}
