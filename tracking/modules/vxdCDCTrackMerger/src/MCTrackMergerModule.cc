/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/MCTrackMergerModule.h>
#include <mdst/dataobjects/MCParticle.h>
#include <vtx/dataobjects/VTXTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(MCTrackMerger)

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

MCTrackMergerModule::MCTrackMergerModule() :
  Module(), m_CDC_wall_radius(16.25)
{
  setDescription(
    "This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("CDCRecoTrackColName", m_CDCRecoTrackColName, "CDC Reco Tracks");
  addParam("VXDRecoTrackColName", m_VXDRecoTrackColName, "VXD Reco Tracks");

  //merging parameters
  addParam("merge_radius", m_merge_radius,
           "Maximum distance between extrapolated tracks on the CDC wall",
           double(2.0));
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

void MCTrackMergerModule::event()
{

  //get CDC tracks
  unsigned int nCDCTracks = m_CDCRecoTracks.getEntries();
  B2DEBUG(9, "MCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  m_totalCDCTracks += nCDCTracks;

  //get VXD tracks
  unsigned int nVXDTracks = m_VXDRecoTracks.getEntries();
  B2DEBUG(9,
          "MCTrackMerger: input Number of VXD Tracks: " << nVXDTracks);
  m_totalVXDTracks += nVXDTracks;

  // Skip in the case there are no MC particles present.
  if (not m_mcParticlesPresent) {
    B2DEBUG(9, "Skipping MC Track Finder as there are no MC Particles registered in the DataStore.");
    return;
  }

  //get MC particles
  StoreArray<MCParticle> mcparticles;

  // Find a MCParticle for each track candidate
  std::vector<int> vxdTrackMCParticles;
  std::vector<int> cdcTrackMCParticles;

  // Find minimum time of flight for each track candidate
  std::vector<double> vxdTrackMinToF;
  std::vector<double> cdcTrackMinToF;

  B2DEBUG(9, "VXD tracks");
  for (auto& recoTrack : m_VXDRecoTracks) {

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
    }

    auto pxdHits = recoTrack.getSortedPXDHitList();
    for (auto& pxdHit : pxdHits) {
      const RelationVector<MCParticle>& relatedMCParticles = pxdHit->getRelationsTo<MCParticle>();
      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }
    }

    auto svdHits = recoTrack.getSortedSVDHitList();
    for (auto& svdHit : svdHits) {
      const RelationVector<MCParticle>& relatedMCParticles = svdHit->getRelationsTo<MCParticle>();
      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
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
      vxdTrackMCParticles.push_back(-1);
      vxdTrackMinToF.push_back(minGlobalTime);
      recoTrack.setQualityIndicator(0.0);
      m_fakeVXDTracks += 1;
    } else if (float(dst.crbegin()->first) / nHits < 0.66)  {
      B2DEBUG(9, "Less than 66% of hits from same MCParticle => fake");
      vxdTrackMCParticles.push_back(-1);
      vxdTrackMinToF.push_back(minGlobalTime);
      recoTrack.setQualityIndicator(0.0);
      m_fakeVXDTracks += 1;
    } else {
      B2DEBUG(9, "MC particle found at " << dst.crbegin()->second);
      vxdTrackMCParticles.push_back(dst.crbegin()->second);
      vxdTrackMinToF.push_back(minGlobalTime - mcparticles[dst.crbegin()->second]->getProductionTime());
      recoTrack.setQualityIndicator(1.0);
      B2DEBUG(9, "GOOD VXD track with min tof " << minGlobalTime - mcparticles[dst.crbegin()->second]->getProductionTime());
    }
  }

  B2DEBUG(9, "CDC tracks");
  for (auto& recoTrack : m_CDCRecoTracks) {

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
    }

    auto svdHits = recoTrack.getSortedSVDHitList();
    for (auto& svdHit : svdHits) {

      const RelationVector<MCParticle>& relatedMCParticles = svdHit->getRelationsTo<MCParticle>();

      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }
    }

    auto vtxHits = recoTrack.getSortedVTXHitList();
    for (auto& vtxHit : vtxHits) {
      const RelationVector<MCParticle>& relatedMCParticles = vtxHit->getRelationsTo<MCParticle>();

      for (size_t i = 0; i < relatedMCParticles.size(); ++i) {
        auto aParticle = relatedMCParticles.object(i);
        contributingMCParticles.push_back(aParticle->getArrayIndex());
      }
    }

    std::map<int, int> counters;
    for (auto i : contributingMCParticles) {
      ++counters[i];
    }

    std::multimap<int, int> dst = flip_map(counters);

    if (dst.size() == 0) {
      B2DEBUG(9, "No MC particle found");
      cdcTrackMCParticles.push_back(-1);
      cdcTrackMinToF.push_back(minGlobalTime);
      recoTrack.setQualityIndicator(0.0);
      m_fakeCDCTracks += 1;
    } else if (float(dst.crbegin()->first) / nHits < 0.66)  {
      B2DEBUG(9, "Less than 66% of hits from same MCParticle => fake");
      cdcTrackMCParticles.push_back(-1);
      cdcTrackMinToF.push_back(minGlobalTime);
      recoTrack.setQualityIndicator(0.0);
      m_fakeCDCTracks += 1;
    } else {
      B2DEBUG(9, "MC particle found at " << dst.crbegin()->second);
      cdcTrackMCParticles.push_back(dst.crbegin()->second);
      cdcTrackMinToF.push_back(minGlobalTime - mcparticles[dst.crbegin()->second]->getProductionTime());
      recoTrack.setQualityIndicator(1.0);
      B2DEBUG(9, "GOOD CDC track with min tof " << minGlobalTime - mcparticles[dst.crbegin()->second]->getProductionTime());
    }
  }

  // Fit all cdc and vxd tracks
  TrackFitter fitter;
  for (auto& cdcTrack : m_CDCRecoTracks) {
    fitter.fit(cdcTrack);
  }
  for (auto& vxdTrack : m_VXDRecoTracks) {
    fitter.fit(vxdTrack);
  }


  B2DEBUG(9, "Matching");
  for (auto& cdcTrack : m_CDCRecoTracks) {
    B2DEBUG(9, "Match with CDCTrack at " <<  cdcTrack.getArrayIndex());

    // get index of matched MCParticle
    int cdcMCParticle = cdcTrackMCParticles[cdcTrack.getArrayIndex()];

    // get min tof of CDC track
    double cdcMinTof = cdcTrackMinToF[cdcTrack.getArrayIndex()];

    // skip CDC Tracks which were not properly fitted
    // TODO: do we want this.
    // TODO: maybe other cuts on hits or whatever
    if (!cdcTrack.wasFitSuccessful())
      continue;

    B2DEBUG(9, "Fitable ");

    // skip CDC if it has already a match
    // TODO: can we allow multiple matches
    if (cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)) {
      m_foundRelatedTracksCDC += 1;

      if ((vxdTrackMCParticles[cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)->getArrayIndex()] == cdcMCParticle) &&
          (cdcMCParticle >= 0))  {
        m_foundCorrectlyRelatedTracksCDC += 1;
      } else {
        if (vxdTrackMCParticles[cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)->getArrayIndex()] < 0)
          m_foundWronglyRelatedTracksCDC_FAKE += 1;
        else
          m_foundWronglyRelatedTracksCDC_OTHER += 1;
      }
      // Comment this out to allow additional relations created by this module
      continue;
    }

    B2DEBUG(9, "Not yet related ");

    bool matched_track = false;

    // TODO: this can be done independent of each other ....
    int currentVxdTrack = -1;
    int bestMatchedVxdTrack = 0;
    for (auto& vxdTrack : m_VXDRecoTracks) {
      B2DEBUG(9, "Compare with  " <<  vxdTrack.getArrayIndex());
      currentVxdTrack++;
      // skip VXD Tracks which were not properly fitted
      if (!vxdTrack.wasFitSuccessful()) {
        continue;
      }

      // skip VXD if it has already a match
      if (vxdTrack.getRelated<RecoTrack>(m_CDCRecoTrackColName)) {
        // Comment this out to account for bad relations
        continue;
      }

      if ((vxdTrackMCParticles[vxdTrack.getArrayIndex()] == cdcTrackMCParticles[cdcTrack.getArrayIndex()]) &&
          (vxdTrackMCParticles[vxdTrack.getArrayIndex()] >= 0))  {
        matched_track = true;
        B2DEBUG(9, "matched to MC particle at: " << vxdTrackMCParticles[vxdTrack.getArrayIndex()]);
        bestMatchedVxdTrack = currentVxdTrack;
      }
    }    //end loop on VXD tracks

    if (matched_track) {
      // -1 is the convention for "before the CDC track" in the related tracks combiner
      B2DEBUG(9, "found match at " << bestMatchedVxdTrack);
      m_VXDRecoTracks[bestMatchedVxdTrack]->addRelationTo(&cdcTrack, -1);
      m_matchedTotal += 1;
    }
  }

  B2DEBUG(9, "Removing curlers in CDC => treat as fake");
  for (auto& cdcTrack : m_CDCRecoTracks) {
    B2DEBUG(9, "Match with CDCTrack at " <<  cdcTrack.getArrayIndex());

    // get index of matched MCParticle
    int cdcMCParticle = cdcTrackMCParticles[cdcTrack.getArrayIndex()];

    // get min tof of CDC track
    double cdcMinTof = cdcTrackMinToF[cdcTrack.getArrayIndex()];

    // skip CDC Tracks which were not properly fitted
    // TODO: do we want this.
    // TODO: maybe other cuts on hits or whatever
    if (!cdcTrack.wasFitSuccessful())
      continue;

    B2DEBUG(9, "Fitable ");

    // skip CDC if it has already a match
    // TODO: can we allow multiple matches
    if (cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)) {
      // Comment this out to account for wrong existing relations and try to add new in this module.
      continue;
    }

    B2DEBUG(9, "Not yet related ");

    for (auto& cdcTrack2 : m_CDCRecoTracks) {
      B2DEBUG(9, "Compare with  " <<  cdcTrack2.getArrayIndex());

      // get min tof of CDC track
      double cdcMinTof2 = cdcTrackMinToF[cdcTrack2.getArrayIndex()];

      // skip self connections
      if (cdcTrack.getArrayIndex() == cdcTrack2.getArrayIndex()) {
        continue;
      }

      // skip CDC tracks which were not properly fitted
      if (!cdcTrack2.wasFitSuccessful()) {
        continue;
      }

      // skip CDC track if it has already a match
      if (cdcTrack2.getRelated<RecoTrack>(m_VXDRecoTrackColName)) {
        continue;
      }

      if ((cdcTrackMCParticles[cdcTrack2.getArrayIndex()] == cdcTrackMCParticles[cdcTrack.getArrayIndex()]) &&
          (cdcTrackMCParticles[cdcTrack.getArrayIndex()] >= 0))  {
        B2DEBUG(9, "matched to MC particle at: " << cdcTrackMCParticles[cdcTrack2.getArrayIndex()]);

        if (cdcMinTof2 < cdcMinTof) {
          cdcTrack.setQualityIndicator(0);
        } else {
          cdcTrack2.setQualityIndicator(0);
        }
        m_removedCDCCurlers += 1;
      }
    }    //end loop on CDC tracks
  }

  B2DEBUG(9, "Removing curlers in VXD => treat as fake");
  for (auto& vxdTrack : m_VXDRecoTracks) {
    B2DEBUG(9, "Match with VXDTrack at " <<  vxdTrack.getArrayIndex());

    // get index of matched MCParticle
    int vxdMCParticle = vxdTrackMCParticles[vxdTrack.getArrayIndex()];

    // get min tof of VXD track
    double vxdMinTof = vxdTrackMinToF[vxdTrack.getArrayIndex()];

    // skip VXD Tracks which were not properly fitted
    // TODO: do we want this.
    // TODO: maybe other cuts on hits or whatever
    if (!vxdTrack.wasFitSuccessful())
      continue;

    B2DEBUG(9, "Fitable ");

    // skip VXD if it has already a match
    // TODO: can we allow multiple matches
    if (vxdTrack.getRelated<RecoTrack>(m_CDCRecoTrackColName)) {
      // comment that out to deal with wrong relations
      continue;
    }

    for (auto& vxdTrack2 : m_VXDRecoTracks) {
      B2DEBUG(9, "Compare with  " <<  vxdTrack2.getArrayIndex());

      // get min tof of VXD track
      double vxdMinTof2 = vxdTrackMinToF[vxdTrack2.getArrayIndex()];

      // skip self connections
      if (vxdTrack.getArrayIndex() == vxdTrack2.getArrayIndex()) {
        // comment that out to deal with wrong relations
        continue;
      }

      // skip VXD Tracks which were not properly fitted
      if (!vxdTrack2.wasFitSuccessful()) {
        continue;
      }

      // skip VXD if it has already a match
      if (vxdTrack2.getRelated<RecoTrack>(m_CDCRecoTrackColName)) {
        continue;
      }

      if ((vxdTrackMCParticles[vxdTrack2.getArrayIndex()] == vxdTrackMCParticles[vxdTrack.getArrayIndex()]) &&
          (vxdTrackMCParticles[vxdTrack.getArrayIndex()] >= 0))  {

        B2DEBUG(9, "found pair of VXD tracks matched to same MC with tof1= " << vxdMinTof << " tof2=" << vxdMinTof2);

        if (vxdMinTof2 < vxdMinTof) {
          vxdTrack.setQualityIndicator(0);
        } else {
          vxdTrack2.setQualityIndicator(0);
        }
        m_removedVXDCurlers += 1;
      }
    }    //end loop on VXD tracks
  }

}

void MCTrackMergerModule::endRun()
{
  B2INFO("The MCTrackMerger processed total of " << m_totalVXDTracks << " VXD track candidates");
  B2INFO("The MCTrackMerger processed total of " << m_totalCDCTracks << " CDC track candidates");
  B2INFO("The MCTrackMerger found total of " <<  m_fakeVXDTracks << " fake VXD track candidates");
  B2INFO("The MCTrackMerger found total of " <<  m_fakeCDCTracks << " fake CDC track candidates");
  B2INFO("The MCTrackMerger found total of " <<  m_foundRelatedTracksCDC << " CDC track candidates with a relation");
  B2INFO("The MCTrackMerger found total of " <<  m_foundCorrectlyRelatedTracksCDC << " CDC track candidates with a correct relation");
  B2INFO("The MCTrackMerger found total of " << m_foundWronglyRelatedTracksCDC_FAKE << " CDC tracks with bad relation to fake track");
  B2INFO("The MCTrackMerger found total of " << m_foundWronglyRelatedTracksCDC_OTHER <<
         " CDC tracks with bad relation to other signal track");
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedTotal << " track candidates");
  B2INFO("The MCTrackMerger removed total of " << m_removedVXDCurlers << " curling track candidates in VXD");
  B2INFO("The MCTrackMerger removed total of " << m_removedCDCCurlers << " curling track candidates in CDC");
}

