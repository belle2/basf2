/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/MCTrackMergerModule.h>
#include <vtx/dataobjects/VTXTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;

REG_MODULE(MCTrackMerger)

namespace {
  template<typename A, typename B>
  std::pair<B, A> flip_pair(const std::pair<A, B>& p)
  {
    return std::pair<B, A>(p.second, p.first);
  }

  template<typename A, typename B>
  std::multimap<B, A> flip_map(const std::map<A, B>& src)
  {
    std::multimap<B, A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A, B>);
    return dst;
  }
}

MCTrackMergerModule::MCTrackMergerModule() :
  Module()
{
  setDescription(
    "This module merges tracks which are reconstructed, separately, in the silicon VXD and in the CDC");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("CDCRecoTrackColName", m_CDCRecoTrackColName, "CDC Reco Tracks");
  addParam("VXDRecoTrackColName", m_VXDRecoTrackColName, "VXD Reco Tracks");
}

void MCTrackMergerModule::initialize()
{
  m_CDCRecoTracks.isRequired(m_CDCRecoTrackColName);
  m_VXDRecoTracks.isRequired(m_VXDRecoTrackColName);

  m_CDCRecoTracks.registerRelationTo(m_VXDRecoTracks);
  m_VXDRecoTracks.registerRelationTo(m_CDCRecoTracks);

  StoreArray<MCParticle> mcparticles;
  if (mcparticles.isOptional()) {
    m_mcParticlesPresent = true;
  }
}

void MCTrackMergerModule::analyzeTrackArray(
  StoreArray<RecoTrack>& recoTracks,
  const StoreArray<MCParticle>& mcParticles,
  std::vector<int>& trackMCParticles,
  std::vector<double>& trackMinToF)
{

  for (auto& recoTrack : recoTracks) {

    std::vector<int> contributingMCParticles;
    auto nHits = recoTrack.getSortedCDCHitList().size() + \
                 recoTrack.getSortedPXDHitList().size() + \
                 recoTrack.getSortedSVDHitList().size() + \
                 recoTrack.getSortedVTXHitList().size();

    // Minimum global time of all sim hits in track
    double minGlobalTime = std::numeric_limits<double>::max();

    auto cdcHits = recoTrack.getSortedCDCHitList();
    for (auto& cdcHit : cdcHits) {
      const RelationVector<MCParticle>& relatedMCParticles = cdcHit->getRelationsFrom<MCParticle>();
      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }

      const RelationVector<CDCSimHit>& relatedCDCSimHits = cdcHit->getRelationsFrom<CDCSimHit>();
      for (size_t i = 0; i < relatedCDCSimHits.size(); ++i) {
        auto aSimHit = relatedCDCSimHits.object(i);
        double tof = aSimHit->getGlobalTime();
        if (tof < minGlobalTime) {
          minGlobalTime = tof;
        }
      }
    }

    auto pxdHits = recoTrack.getSortedPXDHitList();
    for (auto& pxdHit : pxdHits) {
      const RelationVector<MCParticle>& relatedMCParticles = pxdHit->getRelationsTo<MCParticle>();
      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }

      const RelationVector<PXDTrueHit>& relatedPXDTrueHits = pxdHit->getRelationsTo<PXDTrueHit>();
      for (size_t i = 0; i < relatedPXDTrueHits.size(); ++i) {
        auto aTrueHit = relatedPXDTrueHits.object(i);
        double tof = aTrueHit->getGlobalTime();
        if (tof < minGlobalTime) {
          minGlobalTime = tof;
        }
      }
    }

    auto svdHits = recoTrack.getSortedSVDHitList();
    for (auto& svdHit : svdHits) {
      const RelationVector<MCParticle>& relatedMCParticles = svdHit->getRelationsTo<MCParticle>();
      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }

      const RelationVector<SVDTrueHit>& relatedSVDTrueHits = svdHit->getRelationsTo<SVDTrueHit>();
      for (size_t i = 0; i < relatedSVDTrueHits.size(); ++i) {
        auto aTrueHit = relatedSVDTrueHits.object(i);
        double tof = aTrueHit->getGlobalTime();
        if (tof < minGlobalTime) {
          minGlobalTime = tof;
        }
      }
    }

    auto vtxHits = recoTrack.getSortedVTXHitList();
    for (auto& vtxHit : vtxHits) {
      const RelationVector<MCParticle>& relatedMCParticles = vtxHit->getRelationsTo<MCParticle>();
      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }

      const RelationVector<VTXTrueHit>& relatedVTXTrueHits = vtxHit->getRelationsTo<VTXTrueHit>();
      for (size_t i = 0; i < relatedVTXTrueHits.size(); ++i) {
        auto aTrueHit = relatedVTXTrueHits.object(i);
        double tof = aTrueHit->getGlobalTime();
        if (tof < minGlobalTime) {
          minGlobalTime = tof;
        }
      }
    }

    std::map<int, int> counters;
    for (auto i : contributingMCParticles) {
      ++counters[i];
    }

    std::multimap<int, int> dst = flip_map(counters);

    if (dst.size() == 0) {
      B2DEBUG(9, "No MC particle found => fake");
      trackMCParticles.push_back(-1);
      trackMinToF.push_back(minGlobalTime);
    } else if (float(dst.crbegin()->first) / nHits < 0.66)  {
      B2DEBUG(9, "Less than 66% of hits from same MCParticle => fake");
      trackMCParticles.push_back(-1);
      trackMinToF.push_back(minGlobalTime);
    } else {
      B2DEBUG(9, "MC particle found at " << dst.crbegin()->second);
      trackMCParticles.push_back(dst.crbegin()->second);
      trackMinToF.push_back(minGlobalTime - mcParticles[dst.crbegin()->second]->getProductionTime());
      B2DEBUG(9, "GOOD VXD track with min tof " << minGlobalTime - mcParticles[dst.crbegin()->second]->getProductionTime());
    }
  }

  return;
}

void MCTrackMergerModule::cleanTrackArray(
  StoreArray<RecoTrack>& recoTracks,
  std::vector<int>& trackMCParticles,
  bool isVXD)
{
  for (auto& recoTrack : recoTracks) {
    if (trackMCParticles[recoTrack.getArrayIndex()] == -1) {
      recoTrack.setQualityIndicator(0.0);
      if (isVXD) {
        m_fakeVXDTracks += 1;
      } else {
        m_fakeCDCTracks += 1;
      }
    } else {
      recoTrack.setQualityIndicator(1.0);
    }
  }

  return;
}

void MCTrackMergerModule::checkRelatedTrackArrays(
  StoreArray<RecoTrack>& cdcTracks,
  const std::vector<int>& cdcTrackMCParticles,
  const std::vector<double>& cdcTrackMinToF,
  StoreArray<RecoTrack>& vxdTracks,
  const std::vector<int>& vxdTrackMCParticles,
  const std::vector<double>& vxdTrackMinToF)
{

  for (auto& cdcTrack : cdcTracks) {
    // get index of matched MCParticle
    int cdcMCParticle = cdcTrackMCParticles[cdcTrack.getArrayIndex()];

    // get min tof of CDC track
    double cdcMinTof = cdcTrackMinToF[cdcTrack.getArrayIndex()];

    // check if relations to VXD tracks are correct
    RelationVector<RecoTrack> relatedVXDRecoTracks = cdcTrack.getRelationsWith<RecoTrack>(m_VXDRecoTrackColName);
    m_foundRelatedTracks += relatedVXDRecoTracks.size();

    int offset = 0;
    auto initialSize = relatedVXDRecoTracks.size();
    for (unsigned int index = 0; index < initialSize; ++index) {
      auto relatedIndex = relatedVXDRecoTracks[index - offset]->getArrayIndex();
      auto relatedQI = relatedVXDRecoTracks[index - offset]->getQualityIndicator();
      auto relatedToF = vxdTrackMinToF[relatedIndex];

      if ((vxdTrackMCParticles[relatedIndex] == cdcMCParticle) &&
          (cdcMCParticle >= 0) &&
          (relatedQI > 0.0) &&
          (relatedToF < cdcMinTof))  {

        if (relatedVXDRecoTracks.weight(index - offset) > 0) {
          B2INFO("Related VXD: Found wrong weight: " << relatedVXDRecoTracks.weight(index - offset));
          relatedVXDRecoTracks.setWeight(index - offset, -1);
        }
        m_foundCorrectlyRelatedTracks += 1;
      } else {
        // need to remove the bad relation
        relatedVXDRecoTracks.remove(index - offset);
        m_foundButWrongRelations += 1;
        offset += 1;
      }
    }
  }

  for (auto& vxdTrack : vxdTracks) {
    // get index of matched MCParticle
    int vxdMCParticle = vxdTrackMCParticles[vxdTrack.getArrayIndex()];

    // get min tof of VXD track
    double vxdMinTof = vxdTrackMinToF[vxdTrack.getArrayIndex()];

    // check if relations to CDC tracks are correct
    RelationVector<RecoTrack> relatedCDCRecoTracks = vxdTrack.getRelationsWith<RecoTrack>(m_CDCRecoTrackColName);
    m_foundRelatedTracks += relatedCDCRecoTracks.size();

    int offset = 0;
    auto initialSize = relatedCDCRecoTracks.size();
    for (unsigned int index = 0; index < initialSize; ++index) {
      auto relatedIndex = relatedCDCRecoTracks[index - offset]->getArrayIndex();
      auto relatedQI = relatedCDCRecoTracks[index - offset]->getQualityIndicator();
      auto relatedToF = cdcTrackMinToF[relatedIndex];

      if ((cdcTrackMCParticles[relatedIndex] == vxdMCParticle) &&
          (vxdMCParticle >= 0) &&
          (relatedQI > 0.0) &&
          (vxdMinTof < relatedToF))  {

        if (relatedCDCRecoTracks.weight(index - offset) < 0) {
          B2INFO("Related CDC: Found wrong weight: " << relatedCDCRecoTracks.weight(index - offset));
          relatedCDCRecoTracks.setWeight(index - offset, 1);
        }
        m_foundCorrectlyRelatedTracks += 1;

      } else {
        // need to remove the bad relation
        relatedCDCRecoTracks.remove(index - offset);
        m_foundButWrongRelations += 1;
        offset += 1;
      }
    }
  }

  return;
}

void MCTrackMergerModule::mergeVXDAndCDCTrackArrays(
  StoreArray<RecoTrack>& cdcTracks,
  const std::vector<int>& cdcTrackMCParticles,
  const std::vector<double>& cdcTrackMinToF,
  StoreArray<RecoTrack>& vxdTracks,
  const std::vector<int>& vxdTrackMCParticles,
  const std::vector<double>& vxdTrackMinToF)
{

  for (auto& cdcTrack : cdcTracks) {
    B2DEBUG(9, "Match with CDCTrack at " <<  cdcTrack.getArrayIndex());

    // get min tof of CDC track
    double cdcMinTof = cdcTrackMinToF[cdcTrack.getArrayIndex()];

    // skip CDC Tracks with bad quality indicator
    if (cdcTrack.getQualityIndicator() == 0.0)
      continue;

    // check for new relations to VXD tracks missing so far
    for (auto& vxdTrack : vxdTracks) {

      // get min tof of VXD track
      double vxdMinTof = vxdTrackMinToF[vxdTrack.getArrayIndex()];

      B2DEBUG(9, "Compare with  " <<  vxdTrack.getArrayIndex());

      RelationVector<RecoTrack> relatedCDCRecoTracks = vxdTrack.getRelationsWith<RecoTrack>(m_CDCRecoTrackColName);

      // skip VXD if it has already a correct relation
      bool vxdHasGoodRelation = relatedCDCRecoTracks.size() > 0;
      if (vxdHasGoodRelation)
        continue;

      if ((vxdTrackMCParticles[vxdTrack.getArrayIndex()] == cdcTrackMCParticles[cdcTrack.getArrayIndex()]) &&
          (vxdTrackMCParticles[vxdTrack.getArrayIndex()] >= 0) &&
          (vxdMinTof < cdcMinTof))  {
        B2DEBUG(9, "matched to MC particle at: " << vxdTrackMCParticles[vxdTrack.getArrayIndex()]);
        B2DEBUG(9, "vxd_tof: " << vxdMinTof << " < cdc_tof:" << cdcMinTof);
        // -1 is the convention for "before the CDC track" in the related tracks combiner
        m_VXDRecoTracks[vxdTrack.getArrayIndex()]->addRelationTo(&cdcTrack, -1);
        m_matchedTotal += 1;
      }
    }
  }

  return;
}

void MCTrackMergerModule::removeClonesFromTrackArray(
  StoreArray<RecoTrack>& recoTracks,
  std::vector<int>& tracksMCParticles,
  std::vector<double>& tracksMinToF,
  const std::string& relatedTracksColumnName,
  bool isVXD)
{
  for (auto& recoTrack : recoTracks) {

    // get index of matched MCParticle
    int trackMCParticle_1 = tracksMCParticles[recoTrack.getArrayIndex()];

    // get min tof of track
    double trackMinTof_1 = tracksMinToF[recoTrack.getArrayIndex()];

    // check if track has relations
    const RelationVector<RecoTrack>& relatedRecoTracks_1 = recoTrack.getRelationsWith<RecoTrack>(relatedTracksColumnName);
    bool hasGoodRelation_1 = relatedRecoTracks_1.size() > 0;

    for (auto& recoTrack2 : recoTracks) {

      // do not compare a track against itself
      if (recoTrack.getArrayIndex() == recoTrack2.getArrayIndex()) {
        continue;
      }

      // get index of matched MCParticle
      int trackMCParticle_2 = tracksMCParticles[recoTrack2.getArrayIndex()];

      // get min tof of track
      double trackMinTof_2 = tracksMinToF[recoTrack2.getArrayIndex()];

      // check if track has relations
      const RelationVector<RecoTrack>& relatedRecoTracks_2 = recoTrack2.getRelationsWith<RecoTrack>(relatedTracksColumnName);
      bool hasGoodRelation_2 = relatedRecoTracks_2.size() > 0;

      // do not compare a tracks having common related track
      bool commonRelatedTrack = false;
      if (hasGoodRelation_1 && hasGoodRelation_2) {
        commonRelatedTrack = (recoTrack.getRelated<RecoTrack>(relatedTracksColumnName)->getArrayIndex() ==
                              recoTrack2.getRelated<RecoTrack>(relatedTracksColumnName)->getArrayIndex());
      }
      if (commonRelatedTrack) continue;

      if ((trackMCParticle_2 == trackMCParticle_1) &&
          (trackMCParticle_1 >= 0))  {

        if (trackMinTof_2 < trackMinTof_1) {
          if (recoTrack.getQualityIndicator() > 0) {
            recoTrack.setQualityIndicator(0);
            if (isVXD)
              m_removedVXDClones += 1;
            else
              m_removedCDCClones += 1;
            // Also remove the first related VXD track
            if (recoTrack.getRelated<RecoTrack>(relatedTracksColumnName)) {
              recoTrack.getRelated<RecoTrack>(relatedTracksColumnName)->setQualityIndicator(0);
              if (isVXD)
                m_removedCDCClones += 1;
              else
                m_removedVXDClones += 1;
            }
          }
        } else {
          if (recoTrack2.getQualityIndicator() > 0) {
            recoTrack2.setQualityIndicator(0);
            if (isVXD)
              m_removedVXDClones += 1;
            else
              m_removedCDCClones += 1;
            // Also remove the first related VXD track
            if (recoTrack2.getRelated<RecoTrack>(relatedTracksColumnName)) {
              recoTrack2.getRelated<RecoTrack>(relatedTracksColumnName)->setQualityIndicator(0);
              if (isVXD)
                m_removedCDCClones += 1;
              else
                m_removedVXDClones += 1;
            }
          }
        }
      }
    }
  }
  return;
}

void MCTrackMergerModule::event()
{

  // Get CDC tracks
  unsigned int nCDCTracks = m_CDCRecoTracks.getEntries();
  B2DEBUG(9, "MCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  m_totalCDCTracks += nCDCTracks;

  // Get VXD tracks
  unsigned int nVXDTracks = m_VXDRecoTracks.getEntries();
  B2DEBUG(9,
          "MCTrackMerger: input Number of VXD Tracks: " << nVXDTracks);
  m_totalVXDTracks += nVXDTracks;

  // Skip in the case there are no MC particles present
  if (not m_mcParticlesPresent) {
    B2DEBUG(9, "Skipping MC Track Finder as there are no MC Particles registered in the DataStore.");
    return;
  }

  // Get MC particles
  StoreArray<MCParticle> mcparticles;

  // Find a MCParticle for each track candidate
  std::vector<int> vxdTrackMCParticles;
  std::vector<int> cdcTrackMCParticles;

  // Find minimum time of flight for each track candidate
  std::vector<double> vxdTrackMinToF;
  std::vector<double> cdcTrackMinToF;

  B2DEBUG(9, "Analyze VXD tracks");
  analyzeTrackArray(
    m_VXDRecoTracks,
    mcparticles,
    vxdTrackMCParticles,
    vxdTrackMinToF
  );

  B2DEBUG(9, "Clean VXD tracks from fakes");
  cleanTrackArray(
    m_VXDRecoTracks,
    vxdTrackMCParticles,
    true
  );

  B2DEBUG(9, "Analyze CDC tracks");
  analyzeTrackArray(
    m_CDCRecoTracks,
    mcparticles,
    cdcTrackMCParticles,
    cdcTrackMinToF
  );

  B2DEBUG(9, "Clean CDC tracks from fakes");
  cleanTrackArray(
    m_CDCRecoTracks,
    cdcTrackMCParticles,
    false
  );

  B2DEBUG(9, "Checks the existing relations between CDC and VXD tracks and remove wrong ones");
  checkRelatedTrackArrays(
    m_CDCRecoTracks,
    cdcTrackMCParticles,
    cdcTrackMinToF,
    m_VXDRecoTracks,
    vxdTrackMCParticles,
    vxdTrackMinToF
  );

  B2DEBUG(9, "Merging  CDC to VXD tracks, where VXD track will be added before CDC track");
  mergeVXDAndCDCTrackArrays(
    m_CDCRecoTracks,
    cdcTrackMCParticles,
    cdcTrackMinToF,
    m_VXDRecoTracks,
    vxdTrackMCParticles,
    vxdTrackMinToF
  );

  B2DEBUG(9, "Removing clones in CDC");
  removeClonesFromTrackArray(
    m_CDCRecoTracks,
    cdcTrackMCParticles,
    cdcTrackMinToF,
    m_VXDRecoTrackColName,
    false
  );

  B2DEBUG(9, "Removing clones in VXD");
  removeClonesFromTrackArray(
    m_VXDRecoTracks,
    vxdTrackMCParticles,
    vxdTrackMinToF,
    m_CDCRecoTrackColName,
    true
  );

}

void MCTrackMergerModule::endRun()
{
  B2INFO("The MCTrackMerger processed total of " << m_totalVXDTracks << " VXD track candidates");
  B2INFO("The MCTrackMerger processed total of " << m_totalCDCTracks << " CDC track candidates");
  B2INFO("The MCTrackMerger found total of " <<  m_fakeVXDTracks << " fake VXD track candidates");
  B2INFO("The MCTrackMerger found total of " <<  m_fakeCDCTracks << " fake CDC track candidates");
  B2INFO("The MCTrackMerger found total of " <<  m_foundRelatedTracks << " CDC track candidates with a relation");
  B2INFO("The MCTrackMerger found total of " <<  m_foundCorrectlyRelatedTracks << " CDC track candidates with a correct relation");
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedTotal << " track candidates");
  B2INFO("The MCTrackMerger removed total of " << m_removedVXDClones << " clone track candidates in VXD");
  B2INFO("The MCTrackMerger removed total of " << m_removedCDCClones << " clone track candidates in CDC");
}

