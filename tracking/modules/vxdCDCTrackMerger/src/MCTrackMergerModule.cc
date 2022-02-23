/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/MCTrackMergerModule.h>
#include <mdst/dataobjects/MCParticle.h>

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

template< class THit, class TSimHit>
bool isWithinNLoops(double Bz, const THit* aHit, double nLoops)
{
  // for SVD there are cases with more than one simhit attached
  const RelationVector<TSimHit>& relatedSimHits = aHit->template getRelationsWith<TSimHit>();

  // take the first best simhit with mcParticle attached
  const MCParticle* mcParticle = nullptr;
  const TSimHit* aSimHit = nullptr;
  for (const auto& thisSimHit : relatedSimHits) {
    mcParticle = thisSimHit.template getRelated<MCParticle>();
    aSimHit = &thisSimHit;
    if (mcParticle) break;
  }
  if (not mcParticle or not aSimHit) {
    return false;
  }


  // subtract the production time here in order for this classification to also work
  // for particles produced at times t' > t0
  const double tof = aSimHit->getGlobalTime() - mcParticle->getProductionTime();
  const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;
  const float absMom3D = mcParticle->getMomentum().Mag();

  const double loopLength = 2 * M_PI * absMom3D / (Bz * 0.00299792458);
  const double loopTOF =  loopLength / speed;
  if (tof > loopTOF * nLoops) {
    return false;
  } else {
    return true;
  }
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

  // Find a MCParticle for each track candidate
  std::vector<int> vxdTrackMCParticles;
  std::vector<int> cdcTrackMCParticles;


  B2DEBUG(9, "VXD tracks");
  for (auto& recoTrack : m_VXDRecoTracks) {

    std::vector<int> contributingMCParticles;

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
    }

    std::map<int, int> counters;
    for (auto i : contributingMCParticles) {
      ++counters[i];
    }

    std::multimap<int, int> dst = flip_map(counters);

    if (dst.size() == 0) {
      B2DEBUG(9, "No MC particle found");
      vxdTrackMCParticles.push_back(-1);
      m_fakeVXDTracks += 1;
      recoTrack.setQualityIndicator(0.0);
    } else {
      B2DEBUG(9, "MC found particle at " << dst.crbegin()->second);
      vxdTrackMCParticles.push_back(dst.crbegin()->second);
      recoTrack.setQualityIndicator(1.0);
    }
  }

  B2DEBUG(9, "CDC tracks");
  for (auto& recoTrack : m_CDCRecoTracks) {

    std::vector<int> contributingMCParticles;
    auto nHits = recoTrack.getSortedCDCHitList().size() + \
                 recoTrack.getSortedPXDHitList().size() + \
                 recoTrack.getSortedSVDHitList().size() + \
                 recoTrack.getSortedVTXHitList().size();


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
    }

    std::map<int, int> counters;
    for (auto i : contributingMCParticles) {
      ++counters[i];
    }

    std::multimap<int, int> dst = flip_map(counters);


    if (dst.size() == 0) {
      B2DEBUG(9, "No MC particle found");
      cdcTrackMCParticles.push_back(-1);
      m_fakeCDCTracks += 1;
      recoTrack.setQualityIndicator(0.0);
    } else if (float(dst.crbegin()->first) / nHits < 0.66)  {
      B2DEBUG(9, "Less than 66% of hits from same MCParticle => fake");
      cdcTrackMCParticles.push_back(-1);
      m_fakeCDCTracks += 1;
      recoTrack.setQualityIndicator(0.0);
    } else {
      B2DEBUG(9, "MC particle found at " << dst.crbegin()->second);
      cdcTrackMCParticles.push_back(dst.crbegin()->second);
      recoTrack.setQualityIndicator(1.0);
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
      } else if (vxdTrackMCParticles[cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)->getArrayIndex()] < 0) {
        m_foundWronglyRelatedTracksCDC_FAKE += 1;
      }

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
      m_matchedVTXtoCDC += 1;
    }
  }


  B2DEBUG(9, "Matching CDC to CDC");
  for (auto& cdcTrack : m_CDCRecoTracks) {
    B2DEBUG(9, "Match with CDCTrack at " <<  cdcTrack.getArrayIndex());

    // get index of matched MCParticle
    int cdcMCParticle = cdcTrackMCParticles[cdcTrack.getArrayIndex()];


    // skip CDC Tracks which were not properly fitted
    // TODO: do we want this.
    // TODO: maybe other cuts on hits or whatever
    if (!cdcTrack.wasFitSuccessful())
      continue;

    B2DEBUG(9, "Fitable ");

    // skip CDC if it has already a match
    // TODO: can we allow multiple matches
    if (cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)) {
      continue;
    }

    B2DEBUG(9, "Not yet related ");

    bool matched_track = false;

    // TODO: this can be done independent of each other ....
    int currentCdcTrack = -1;
    int bestMatchedCdcTrack = 0;
    for (auto& cdcTrack2 : m_CDCRecoTracks) {
      B2DEBUG(9, "Compare with  " <<  cdcTrack2.getArrayIndex());
      currentCdcTrack++;

      // skip self connections
      if (cdcTrack.getArrayIndex() == cdcTrack2.getArrayIndex()) {
        continue;
      }

      // skip VXD Tracks which were not properly fitted
      if (!cdcTrack2.wasFitSuccessful()) {
        continue;
      }

      // skip VXD if it has already a match
      if (cdcTrack2.getRelated<RecoTrack>(m_VXDRecoTrackColName)) {
        continue;
      }

      if ((cdcTrackMCParticles[cdcTrack2.getArrayIndex()] == cdcTrackMCParticles[cdcTrack.getArrayIndex()]) &&
          (cdcTrackMCParticles[cdcTrack.getArrayIndex()] >= 0))  {
        matched_track = true;
        B2DEBUG(9, "matched to MC particle at: " << cdcTrackMCParticles[cdcTrack2.getArrayIndex()]);
        bestMatchedCdcTrack = currentCdcTrack;
      }
    }    //end loop on VXD tracks

    if (matched_track) {
      // -1 is the convention for "before the CDC track" in the related tracks combiner
      B2DEBUG(9, "found match at " << bestMatchedCdcTrack);
      //m_CDCRecoTracks[bestMatchedCdcTrack]->addRelationTo(&cdcTrack, -1);
      m_matchedTotal += 1;
      m_matchedCDCtoCDC += 1;
    }
  }

  B2DEBUG(9, "Matching VXD to VXD");
  for (auto& vxdTrack : m_VXDRecoTracks) {
    B2DEBUG(9, "Match with VXDTrack at " <<  vxdTrack.getArrayIndex());

    // get index of matched MCParticle
    int vxdMCParticle = vxdTrackMCParticles[vxdTrack.getArrayIndex()];


    // skip VXD Tracks which were not properly fitted
    // TODO: do we want this.
    // TODO: maybe other cuts on hits or whatever
    if (!vxdTrack.wasFitSuccessful())
      continue;

    B2DEBUG(9, "Fitable ");

    // skip VXD if it has already a match
    // TODO: can we allow multiple matches
    if (vxdTrack.getRelated<RecoTrack>(m_CDCRecoTrackColName)) {
      continue;
    }

    B2DEBUG(9, "Not yet related ");

    bool matched_track = false;

    // TODO: this can be done independent of each other ....
    int currentVxdTrack = -1;
    int bestMatchedVxdTrack = 0;
    for (auto& vxdTrack2 : m_VXDRecoTracks) {
      B2DEBUG(9, "Compare with  " <<  vxdTrack2.getArrayIndex());
      currentVxdTrack++;

      // skip self connections
      if (vxdTrack.getArrayIndex() == vxdTrack2.getArrayIndex()) {
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
        matched_track = true;
        B2DEBUG(9, "matched to MC particle at: " << vxdTrackMCParticles[vxdTrack2.getArrayIndex()]);
        bestMatchedVxdTrack = currentVxdTrack;
      }
    }    //end loop on VXD tracks

    if (matched_track) {
      // -1 is the convention for "before the VXD track" in the related tracks combiner
      B2DEBUG(9, "found match at " << bestMatchedVxdTrack);
      //m_VXDRecoTracks[bestMatchedVxdTrack]->addRelationTo(&vxdTrack, -1);
      m_matchedTotal += 1;
      m_matchedVTXtoVTX += 1;
    }
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
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedTotal << " track candidates");
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedVTXtoCDC << " track candidates from VXD to CDC");
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedVTXtoVTX  << " track candidates from VXD to VXD");
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedCDCtoVTX << " track candidates from CDC to VXD");
  B2INFO("The MCTrackMerger matched total of " <<  m_matchedCDCtoCDC << " track candidates from CDC to CDC");
  B2INFO("The MCTrackMerger tagged total of " << m_removedCurlers << " track candidates with hits beyond first loop");
}

