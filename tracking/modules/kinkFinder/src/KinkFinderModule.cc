/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/kinkFinder/KinkFinderModule.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <genfit/MeasuredStateOnPlane.h>

using namespace Belle2;

REG_MODULE(KinkFinder);

KinkFinderModule::KinkFinderModule() : Module()
{

  setDescription("This is a Kink finder module which preselects mother and daughter candidate tracks and matches them "
                 "and fits a vertex for each pair. Depending on the outcome of each fit, a corresponding "
                 "``Belle2::Kink`` is stored or not.\n\n"
                 "The parameters of the ``KinkFinder`` are stored as payloads in ``KinkFinderParameters``.\n\n"
                 "A loose cut on the pair is applied before the fit; then, a vertex fit is performed, and only pairs "
                 "passing a chi^2 (``KinkFinderParameters::m_vertexChi2Cut``) and distance "
                 "(``KinkFinderParameters::m_vertexDistanceCut``) are stored as ``Belle2::Kink``.\n\n"
                 "If a corresponding ``KinkFitter`` mode is ON, hits are reassigned between mother and daughter tracks "
                 "to improve the resolutions and efficiency. If a corresponding ``KinkFitter`` mode is ON, the track "
                 "pair is also fitted as one track and a special flag is filled based on the result to supress the clones.\n\n"
                 "If a corresponding ``KinkFitter`` mode is ON, ``KinkFinder`` preselects track candidates "
                 "that might be formed from two kink tracks, and ``KinkFitter`` splits such tracks. "
                 "After that the result is stored in ``Belle2::Kink``.");

  setPropertyFlags(c_ParallelProcessingCertified);

  // input: RecoTracks
  addParam("RecoTracks", m_arrayNameRecoTrack,
           "RecoTrack StoreArray name (input)", std::string(""));

  // RecoTracks for the refitting procedures
  addParam("CopiedRecoTracks", m_arrayNameCopiedRecoTrack,
           "RecoTrack StoreArray name (used for track refitting)", std::string("RecoTracksKinkTmp"));

  // input: Tracks and TrackFitResults; output: TrackFitResults (for the refitted tracks)
  addParam("TrackFitResults", m_arrayNameTFResult,
           "Belle2::TrackFitResult StoreArray name (in- and output).\n"
           "Note that the Kinks use pointers indices into these arrays, so all hell may break loose, "
           "if you change this.", std::string(""));
  addParam("Tracks", m_arrayNameTrack,
           "Belle2::Track StoreArray name (input).\n"
           "Note that the Kinks use pointers indices into these arrays, so all hell may break loose, "
           "if you change this.", std::string(""));

  // output: Kinks
  addParam("Kinks", m_arrayNameKink, "Kink StoreArray name (output).", std::string(""));

}


void KinkFinderModule::initialize()
{
  m_tracks.isRequired(m_arrayNameTrack);
  StoreArray<RecoTrack> recoTracks(m_arrayNameRecoTrack);
  m_tracks.requireRelationTo(recoTracks);
  // All the other required StoreArrays are checked in the Constructor of the KinkFitter.

  // Create KinkFitter object.
  // Its parameters and cuts are set at the beginning of each run based on the information from the database.
  m_kinkFitter = std::make_unique<KinkFitter>(m_arrayNameTFResult, m_arrayNameKink,
                                              m_arrayNameRecoTrack,
                                              m_arrayNameCopiedRecoTrack);

  // Set geometry of CDC forward and backward walls.
  // Here the tangent is forward CDC acceptance (17 degrees).
  m_cdcForwardBottomWall.setLine(tan(17. / 180 * M_PI), 0);
  // Here numbers are calculated from the CDC technical geometry.
  m_cdcForwardTopWall.setLine(4.97, -670);
  // Here the tangent is backward CDC acceptance (30 degrees).
  m_cdcBackwardBottomWall.setLine(-tan(30. / 180 * M_PI), 0);
  // Here numbers are calculated from the CDC technical geometry.
  m_cdcBackwardTopWall.setLine(-4.2, -192.5);

}

void KinkFinderModule::beginRun()
{
  if (!m_kinkFinderParameters.isValid())
    B2FATAL("KinkFinder parameters are not available.");

  // apply cuts and fitter mode to KinkFitter
  m_kinkFitter->initializeCuts(m_kinkFinderParameters->getVertexDistanceCut(),
                               m_kinkFinderParameters->getVertexChi2Cut(),
                               m_kinkFinderParameters->getPrecutDistance());
  m_kinkFitter->setFitterMode(m_kinkFinderParameters->getKinkFitterMode());
}


void KinkFinderModule::event()
{
  B2DEBUG(29, m_tracks.getEntries() << " tracks in event.");

  // Group tracks into mother and daughter candidates.
  std::vector<const Track*> tracksMother;
  tracksMother.reserve(m_tracks.getEntries());

  std::vector<const Track*> tracksDaughter;
  tracksDaughter.reserve(m_tracks.getEntries());

  for (const auto& track : m_tracks) {
    RecoTrack const* const recoTrack = track.getRelated<RecoTrack>(m_arrayNameRecoTrack);
    B2ASSERT("No RecoTrack available for given Track.", recoTrack);

    // track is fitted
    if (!recoTrack->hasTrackFitStatus() || !recoTrack->wasFitSuccessful()) continue;

    // exclude tracks that have CDC hits and end with VXD hits (the kink efficiency loss is negligible)
    const auto& recoHitsInformation = recoTrack->getRecoHitInformations(true);
    if (recoTrack->getNumberOfCDCHits() &&
        ((recoHitsInformation.back())->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD ||
         (recoHitsInformation.back())->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_PXD))
      continue;

    bool trackChosen = false;
    // select mother candidates
    if (preFilterMotherTracks(&track)) {
      tracksMother.push_back(&track);
      trackChosen = true;

      // if mother candidate pass criteria for splitting, split it with filter 7
      if (kinkFitterModeSplitTrack() && preFilterTracksToSplit(&track)) {
        const short filterFlag = 7;
        fitAndStore(&track, &track, filterFlag);
      }
    }
    // select daughter candidates
    if (preFilterDaughterTracks(&track)) {
      tracksDaughter.push_back(&track);
      trackChosen = true;

      // if daughter candidate pass criteria for splitting, split it with filter 8
      if (kinkFitterModeSplitTrack() && preFilterTracksToSplit(&track)) {
        const short filterFlag = 8;
        fitAndStore(&track, &track, filterFlag);
      }
    }

    // if track does not pass mother or daughter selection but passes criteria for splitting, split it with filter 9
    if (kinkFitterModeSplitTrack() && !trackChosen && preFilterTracksToSplit(&track)) {
      const short filterFlag = 9;
      fitAndStore(&track, &track, filterFlag);
    }
  }

  // Reject boring events.
  if (tracksMother.empty() || tracksDaughter.empty()) {
    B2DEBUG(29, "No interesting track pairs found. Number of selected tracksMother: " << tracksMother.size() <<
            ", tracksDaughter " << tracksDaughter.size());
    return;
  }

  // Pair up each mother track with each daughter track.
  for (auto& trackMother : tracksMother) {
    for (auto& trackDaughter : tracksDaughter) {
      const short filterFlag = isTrackPairSelected(trackMother, trackDaughter);
      if (!filterFlag) continue;
      B2DEBUG(29, "Found kink candidate with filterFlag " << filterFlag);
      fitAndStore(trackMother, trackDaughter, filterFlag);
    }
  }

}


void KinkFinderModule::terminate()
{
  B2INFO("===KinkFinder summary===========================================================");
  B2INFO("In total " << m_allStored << " kinks saved. Among them");
  B2INFO("track pairs with close endpoints: " << m_f1Stored << ";");
  B2INFO("track pairs with missing layers between their endpoints (Helix extrapolation selection): " << m_f2Stored << ";");
  B2INFO("split track chosen from mother candidates: " << m_f3Stored << ";");
  B2INFO("split track chosen from daughter candidates: " << m_f4Stored << ";");
  B2INFO("split track chosen from tracks not passing mother/daughter preselection " << m_f5Stored << ".");
}

/// Fitter mode 4th bit responsible for turning On/Off track splitting
bool KinkFinderModule::kinkFitterModeSplitTrack()
{
  return m_kinkFinderParameters->getKinkFitterMode() & 0b1000;
}

/// Test if the point in space is inside CDC (approximate custom geometry) with respect to shifts from outer wall.
bool KinkFinderModule::ifInCDC(const ROOT::Math::XYZVector& pos)
{
  // check for the first layer of CDC (needed separately because of calculation of trapezoid shapes later)
  if (pos.Perp2() < m_cdcInnerWallWithoutFirstLayer * m_cdcInnerWallWithoutFirstLayer
      && pos.Perp2() > m_cdcInnerWithFirstLayerWall * m_cdcInnerWithFirstLayerWall)
    return true;

  // position inside CDC with respect to required shifts
  const double z = (pos.Z() > 0) ? (pos.Z() + m_kinkFinderParameters->getPrecutZ()) : (pos.Z() -
                   m_kinkFinderParameters->getPrecutZ());
  const double r = sqrt(pos.Perp2()) + m_kinkFinderParameters->getPrecutRho();

  // the point should be inside CDC with respect to shift, precutRho, from outer wall
  if (!((r < m_cdcOuterWall) && (r > m_cdcInnerWallWithoutFirstLayer + m_kinkFinderParameters->getPrecutRho())))
    return false;

  // the z coordinate point restriction
  // CDC has a shape of two trapezoids; thus, the forward and backward walls are described with two lines: bottom and top
  bool z_part;
  if (z > 0) {
    // forward walls of CDC
    z_part = (r > m_cdcForwardBottomWall.getLine(z)) && (r > m_cdcForwardTopWall.getLine(z));
  } else {
    // backward walls of CDC
    z_part = (r > m_cdcBackwardBottomWall.getLine(z)) && (r > m_cdcBackwardTopWall.getLine(z));
  }
  return z_part;
}

/// Check if the track can be a mother candidate based on some simple selections.
/// They are conservative enough to reduces the combinatorial background and save as much signal events as possible.
bool KinkFinderModule::preFilterMotherTracks(Track const* const track)
{
  // RecoTrack (existence and fit checked before calling this function)
  RecoTrack const* const recoTrack = track->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  // track fit result
  TrackFitResult const* const trackFitResult = track->getTrackFitResultWithClosestMass(Const::pion);

  // total number of CDC hits
  const int nCDCHits = recoTrack->getNumberOfCDCHits();

  // last point of the track is > precutRho and > precutZ from outer walls of CDC
  const ROOT::Math::XYZVector posLast(recoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  if (nCDCHits && !ifInCDC(posLast)) return false;

  // track is not curled back to IP
  // The first layer of CDC has 16.8 cm radius; thus, the track which ends inside m_cdcInnerWallWithoutFirstLayer=17 cm
  // cylinder is either short or curler.
  // The requirement of >= 10 hits ensures that it is a curler (obtained empirically)
  if (nCDCHits >= 10 && posLast.Perp2() < m_cdcInnerWallWithoutFirstLayer * m_cdcInnerWallWithoutFirstLayer) return false;

  // first point of the track is inside inner layers of VXD
  // The mother candidate should start inside VXD. The radius of outer two SVD layers are 10.4 and 13.5 cm,
  // so we set m_svdBeforeOuterLayer=12 cm cut (between these layers).
  const ROOT::Math::XYZVector posFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if (posFirst.Perp2() > m_svdBeforeOuterLayer * m_svdBeforeOuterLayer) return false;

  // impact parameter in rphi < 1 cm
  // conservative enough assuming possible bad resolution of mother track due to daughter hits assignment
  if (fabs(trackFitResult->getD0()) > 1) return false;
  return true;
}

/// Check if the track can be a daughter candidate based on some simple selections.
/// They are conservative enough to reduces the combinatorial background and save as much signal events as possible.
bool KinkFinderModule::preFilterDaughterTracks(Track const* const track)
{
  // RecoTrack (existence and fit checked before calling this function)
  RecoTrack const* const recoTrack = track->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  // track fit result
  TrackFitResult const* const trackFitResult = track->getTrackFitResultWithClosestMass(Const::pion);

  // first and last points of the track are > precutRho and > precutZ from outer walls of CDC (against back splashes)
  const ROOT::Math::XYZVector posLast(recoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  const ROOT::Math::XYZVector posFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if (!ifInCDC(posLast) && !ifInCDC(posFirst)) return false;

  // first point of the track is outside inner layers of VXD, or impact parameter in rphi > 1 cm
  // inverse criteria for mother
  if (posFirst.Perp2() < m_svdBeforeOuterLayer * m_svdBeforeOuterLayer && fabs(trackFitResult->getD0()) < 1) return false;
  return true;
}

/// Check if the track is a candidate for splitting based on some simple selections.
/// They are optimized to reduce the splitting of ordinary (non-decayed) tracks.
bool KinkFinderModule::preFilterTracksToSplit(Track const* const track)
{
  // RecoTrack (existence and fit checked before calling this function)
  RecoTrack const* const recoTrack = track->getRelated<RecoTrack>(m_arrayNameRecoTrack);

  if (recoTrack->getTrackFitStatus()->getPVal() > m_kinkFinderParameters->getPrecutSplitPValue()) return false;

  // track fit result
  TrackFitResult const* const trackFitResult = track->getTrackFitResultWithClosestMass(Const::pion);

  // number of fitted CDC hits
  const int nFittedCDCHits = trackFitResult->getHitPatternCDC().getNHits();
  if (nFittedCDCHits > m_kinkFinderParameters->getPrecutSplitNCDCHit()) return false;
  // more than 5 CDC hits to have enough hits for possible splitting
  if (nFittedCDCHits < 5) return false;

  // first point of the track is close to the inner wall of the CDC
  // we set m_cdcInnerWallWithoutFirstTwoLayers=18 cm cut to have both tracks with and without VXD hits
  const ROOT::Math::XYZVector posFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if (posFirst.Perp2() > m_cdcInnerWallWithoutFirstTwoLayers * m_cdcInnerWallWithoutFirstTwoLayers) return false;

  // impact parameter in rphi < 2 cm
  // Loose enough assuming possible bad resolution of track due to daughter hits assignment
  if (fabs(trackFitResult->getD0()) > 2) return false;

  return true;
}

/// Track pair preselection based on distance between two tracks with six different options.
short KinkFinderModule::isTrackPairSelected(const Track* motherTrack, const Track* daughterTrack)
{
  constexpr double M_PI_over_6 = M_PI / 6.;
  const double cos_M_PI_over_6 = cos(M_PI_over_6);

  // cut variables squared for convenience
  const double precutDistanceSquared = m_kinkFinderParameters->getPrecutDistance() * m_kinkFinderParameters->getPrecutDistance();
  const double precutDistance2DSquared = m_kinkFinderParameters->getPrecutDistance2D() *
                                         m_kinkFinderParameters->getPrecutDistance2D();

  // get recoTracks and check that they are not the same
  RecoTrack const* const motherRecoTrack = motherTrack->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  RecoTrack const* const daughterRecoTrack = daughterTrack->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  if (motherRecoTrack == daughterRecoTrack) return 0;

  // Filter 1 check.
  const ROOT::Math::XYZVector motherPosLast(motherRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  const ROOT::Math::XYZVector daughterPosFirst(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if ((daughterPosFirst - motherPosLast).Mag2() < precutDistanceSquared) return 1;

  // Filter 2 check.
  const ROOT::Math::XYZVector daughterPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  if ((daughterPosLast - motherPosLast).Mag2() < precutDistanceSquared) return 2;

  // Filter 3 check. Here the same direction of daughter and mother is checked to exclude intersection of tracks
  // from different hemispheres.
  // Here and further M_PI_over_6 criteria was obtained empirically: it conservatively reduces the combinatorial
  // background while saving as much signal events as possible.
  if (daughterPosLast.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      daughterPosFirst.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      fabs(daughterPosLast.Phi() - motherPosLast.Phi()) < M_PI_over_6 ||
      fabs(daughterPosFirst.Phi() - motherPosLast.Phi()) < M_PI_over_6) {

    const ROOT::Math::XYZVector daughterPosClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
    const ROOT::Math::XYZVector daughterMomClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
    const double Bz = BFieldManager::getFieldInTesla(daughterPosClosestToMotherPosLast).Z();
    // daughter Helix with move to mother last point
    Helix daughterHelixClosestToMotherPosLast(daughterPosClosestToMotherPosLast,
                                              daughterMomClosestToMotherPosLast,
                                              static_cast<short>(daughterRecoTrack->getTrackFitStatus()->getCharge()),
                                              Bz);
    daughterHelixClosestToMotherPosLast.passiveMoveBy(motherPosLast);

    if ((daughterHelixClosestToMotherPosLast.getD0() * daughterHelixClosestToMotherPosLast.getD0() +
         daughterHelixClosestToMotherPosLast.getZ0() * daughterHelixClosestToMotherPosLast.getZ0()) <
        precutDistanceSquared)
      return 3;
  }

  // Filter 4 check.
  if ((daughterPosFirst - motherPosLast).Perp2() < precutDistance2DSquared)
    return 4;

  // Filter 5 check.
  if ((daughterPosLast - motherPosLast).Perp2() < precutDistance2DSquared)
    return 5;

  // Filter 6 check. Here the same direction of daughter and mother is checked to exclude intersection of tracks
  // from different hemispheres.
  if (daughterPosLast.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      daughterPosFirst.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      fabs(daughterPosLast.Phi() - motherPosLast.Phi()) < M_PI_over_6 ||
      fabs(daughterPosFirst.Phi() - motherPosLast.Phi()) < M_PI_over_6) {

    const ROOT::Math::XYZVector daughterPosClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
    const ROOT::Math::XYZVector daughterMomClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
    const double Bz = BFieldManager::getFieldInTesla(daughterPosClosestToMotherPosLast).Z();
    // daughter Helix with move to mother last point
    Helix daughterHelixClosestToMotherPosLast(daughterPosClosestToMotherPosLast,
                                              daughterMomClosestToMotherPosLast,
                                              static_cast<short>(daughterRecoTrack->getTrackFitStatus()->getCharge()),
                                              Bz);
    daughterHelixClosestToMotherPosLast.passiveMoveBy(motherPosLast);

    if (fabs(daughterHelixClosestToMotherPosLast.getD0()) < m_kinkFinderParameters->getPrecutDistance())
      return 6;
  }

  // No filter is passed.
  return 0;

}

/// Kink fitting and storing.
void KinkFinderModule::fitAndStore(const Track* trackMother, const Track* trackDaughter, const short filterFlag)
{
  bool ok = m_kinkFitter->fitAndStore(trackMother, trackDaughter, filterFlag);
  if (ok) {
    ++m_allStored;
    switch (filterFlag) {
      case 1:
      case 2:
      case 4:
      case 5:
        ++m_f1Stored;
        break;
      case 3:
      case 6:
        ++m_f2Stored;
        break;
      case 7:
        ++m_f3Stored;
        break;
      case 8:
        ++m_f4Stored;
        break;
      case 9:
        ++m_f5Stored;
    }
  }
}
