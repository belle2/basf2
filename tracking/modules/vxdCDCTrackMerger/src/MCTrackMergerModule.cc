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

  //get VXD tracks
  unsigned int nVXDTracks = m_VXDRecoTracks.getEntries();
  B2DEBUG(9,
          "MCTrackMerger: input Number of VXD Tracks: " << nVXDTracks);

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
      B2DEBUG(9, "No MC found particle found");
      vxdTrackMCParticles.push_back(-1);
    } else {
      B2DEBUG(9, "MC found particle found at " << dst.crbegin()->second);
      vxdTrackMCParticles.push_back(dst.crbegin()->second);
    }
  }

  B2DEBUG(9, "CDC tracks");
  for (auto& recoTrack : m_CDCRecoTracks) {

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
      B2DEBUG(9, "No MC found particle found");
      cdcTrackMCParticles.push_back(-1);
    } else {
      B2DEBUG(9, "MC found particle found at " << dst.crbegin()->second);
      cdcTrackMCParticles.push_back(dst.crbegin()->second);
    }
  }

  // position and momentum used for extrapolations to the CDC Wall
  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  // position and momentum of the track extrapolated from the CDC fit to the CDC Wall
  TVector3 cdcpos;
  TVector3 cdcmom;
  // position and momentum of the track extrapolated from the VXD fit to the CDC Wall
  TVector3 vxdpos;
  TVector3 vxdmom;

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

    // start search with worst chi^2 imaginable
    double this_chi_2 = std::numeric_limits<double>::max();
    double chi_2_max_this_cdc_track = std::numeric_limits<double>::max();
    try {
      genfit::MeasuredStateOnPlane cdc_sop =
        cdcTrack.getMeasuredStateOnPlaneFromFirstHit();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      cdcpos = cdc_sop.getPos();
      cdcmom = cdc_sop.getMom();
    } catch (...) {
      B2DEBUG(9, "CDCTrack extrapolation to cylinder failed!");
      continue;
    }

    B2DEBUG(9, "Extrapolatable ");


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

      try {
        genfit::MeasuredStateOnPlane vxd_sop =
          vxdTrack.getMeasuredStateOnPlaneFromLastHit();
        vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
      } catch (genfit::Exception const&) {
        // extrapolation not possible, skip this track
        B2DEBUG(9, "VXDTrack extrapolation to cylinder failed!");
        continue;
      }

      if ((vxdTrackMCParticles[vxdTrack.getArrayIndex()] == cdcTrackMCParticles[cdcTrack.getArrayIndex()]) &&
          (vxdTrackMCParticles[vxdTrack.getArrayIndex()] >= 0))  {
        matched_track = true;
        B2DEBUG(9, "matched to MC particle at: " << vxdTrackMCParticles[vxdTrack.getArrayIndex()]);
        bestMatchedVxdTrack = currentVxdTrack;
      }

      /*
      // Extrapolate tracks to same plane & Match Tracks
      if (TMath::Sqrt((cdcpos - vxdpos) * (cdcpos - vxdpos)) < m_merge_radius) {
        genfit::MeasuredStateOnPlane vxd_sop;
        genfit::MeasuredStateOnPlane cdc_sop;
        try {
          vxd_sop = vxdTrack.getMeasuredStateOnPlaneFromLastHit();
          cdc_sop = cdcTrack.getMeasuredStateOnPlaneFromFirstHit();
          cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
        } catch (genfit::Exception const&) {
          // extrapolation not possible, skip this track
          B2DEBUG(9, "VXDTrack extrapolation to plane failed!");
          continue;
        }

        try {
          TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
          TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
          state_diff *= inv_covmtrx;
          this_chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());
        } catch (...) {
          B2WARNING("Matrix is singular!");
          continue;
        }

        if ((this_chi_2 < chi_2_max_this_cdc_track) && (this_chi_2 > 0)) {
          matched_track = true;
          chi_2_max_this_cdc_track = this_chi_2;
          bestMatchedVxdTrack = currentVxdTrack;
        }
      }    //If on radius
      */
    }    //end loop on VXD tracks

    if (matched_track) {
      // -1 is the convention for "before the CDC track" in the related tracks combiner
      B2DEBUG(9, "found match at " << bestMatchedVxdTrack);
      m_VXDRecoTracks[bestMatchedVxdTrack]->addRelationTo(&cdcTrack, -1);
    }
  }
}
