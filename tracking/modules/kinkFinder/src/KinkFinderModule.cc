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

#include <genfit/MeasuredStateOnPlane.h>

using namespace Belle2;

REG_MODULE(KinkFinder);

KinkFinderModule::KinkFinderModule() : Module()
{

  setDescription("This is a simple Kink finder"
                 "which matches all mother candidate tracks with all daughter candidate tracks, "
                 "fitting a vertex for each pair. "
                 "Depending on the outcome of each fit, a corresponding "
                 "Belle2::Kink is stored or not.\n\n"
                 "A loose cut on the pair is applied before the fit; "
                 "then, a vertex fit is performed, and only pairs passing a chi^2 (``vertexChi2Cut``) "
                 "and distance (``vertexDistanceCut``) "
                 "are stored as Belle2::Kink.");

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
  //All the other required StoreArrays are checked in the Construtor of the kinkFitter.

  if (!m_kinkFinderParameters.isValid())
    B2FATAL("kinkFinder parameters are not available.");
  // mode of the kinkFitter
  m_kinkFitterMode = m_kinkFinderParameters->getKinkFitterMode();
  m_kinkFitterModeSplitTrack = m_kinkFitterMode & 0b1000;

  // preselection cuts
  m_precutRho = m_kinkFinderParameters->getPrecutRho();
  m_precutZ = m_kinkFinderParameters->getPrecutZ();
  m_precutDistance = m_kinkFinderParameters->getPrecutDistance();
  m_precutDistance2D = m_kinkFinderParameters->getPrecutDistance2D();
  m_precutDistanceSquared = m_precutDistance * m_precutDistance;
  m_precutDistance2DSquared = m_precutDistance2D * m_precutDistance2D;
  m_precutSplitNCDCHit = m_kinkFinderParameters->getPrecutSplitNCDCHit();
  m_precutSplitPValue = m_kinkFinderParameters->getPrecutSplitPValue();

  // final cuts used in kinkFitter to decide if the kink candidate should be stored
  m_vertexChi2Cut = m_kinkFinderParameters->getVertexChi2Cut();
  m_vertexDistanceCut = m_kinkFinderParameters->getVertexDistanceCut();


  m_kinkFitter = std::make_unique<kinkFitter>(m_arrayNameTFResult, m_arrayNameKink,
                                              m_arrayNameRecoTrack,
                                              m_arrayNameCopiedRecoTrack);
  m_kinkFitter->initializeCuts(m_vertexDistanceCut, m_vertexChi2Cut, m_precutDistance);
  m_kinkFitter->setFitterMode(m_kinkFitterMode);

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

    bool trackChosen = false;
    if (preFilterMotherTracks(recoTrack)) {
      tracksMother.push_back(&track);
      trackChosen = true;

      if (m_kinkFitterModeSplitTrack && preFilterTracksToSplit(recoTrack, &track)) {
        const short filterFlag = 7;
        fitAndStore(&track, &track, filterFlag);
      }
    }
    if (preFilterDaughterTracks(recoTrack)) {
      tracksDaughter.push_back(&track);
      trackChosen = true;

      if (m_kinkFitterModeSplitTrack && preFilterTracksToSplit(recoTrack, &track)) {
        const short filterFlag = 8;
        fitAndStore(&track, &track, filterFlag);
      }
    }

    if (m_kinkFitterModeSplitTrack && !trackChosen && preFilterTracksToSplit(recoTrack, &track)) {
      const short filterFlag = 9;
      fitAndStore(&track, &track, filterFlag);
    }
  }

  // Try to find kink among mother candidates and reject boring events.
  if (tracksMother.empty() || tracksDaughter.empty()) {

    // Try to find kink among mother candidates.
    bool foundAmongMothersFlag = false;
    if (tracksDaughter.empty() && tracksMother.size() > 1) {
      for (auto itTrackMother = tracksMother.begin(); itTrackMother != tracksMother.end(); ++itTrackMother) {
        for (auto itTrackMother2 = std::next(itTrackMother); itTrackMother2 != tracksMother.end(); ++itTrackMother2) {

          const short filterFlag = isTrackPairSelected(*itTrackMother, *itTrackMother2);
          if (filterFlag != 2) continue;
          B2DEBUG(29, "Found kink candidate among mothers with filterFlag " << filterFlag);
          foundAmongMothersFlag = true;
          // Particle with large momentum (smaller curvature) should be mother.
          if ((*itTrackMother)->getTrackFitResultWithClosestMass(Const::pion)->getOmega() <
              (*itTrackMother2)->getTrackFitResultWithClosestMass(Const::pion)->getOmega())
            fitAndStore(*itTrackMother, *itTrackMother2, filterFlag);
          else
            fitAndStore(*itTrackMother2, *itTrackMother, filterFlag);
        }
      }
    }
    // Reject boring events.
    if (!foundAmongMothersFlag) {
      B2DEBUG(29, "No interesting track pairs. tracksMother " << tracksMother.size() << ", tracksDaughter "
              << tracksDaughter.size());
    }
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
  B2INFO("In total " << m_allStored << " kinks saved.");
  B2INFO("Among them with filter1 " << m_f1Stored << ", f2 " << m_f2Stored << ", f3 " << m_f3Stored << ", f4 "
         << m_f4Stored << ", f5 " << m_f5Stored);
}

/// Test if the point in space is inside CDC (approximate custom geometry) with respect to shifts from outer wall.
bool KinkFinderModule::ifInCDC(ROOT::Math::XYZVector& pos)
{

  double z = (pos.Z() > 0) ? (pos.Z() + m_precutZ) : (pos.Z() - m_precutZ);
  double r = sqrt(pos.Perp2()) + m_precutRho;

  bool r_part = (r < 112) && (r > 17 + m_precutRho);
  bool z_part;
  if (z > 0) {
    z_part = (r > (tan(17. / 180 * M_PI) * z)) && (r > (4.97 * z - 670));
  } else {
    z_part = (r > (-tan(30. / 180 * M_PI) * z)) && (r > (-4.2 * z - 192.5));
  }
  return ((r_part && z_part) || (pos.Perp2() < 289 && pos.Perp2() > 225));
}

/// Check if the track can be a mother candidate based on some simple selections.
/// They are conservative enough to reduces the combinatorial background and save as much signal events as possible.
bool KinkFinderModule::preFilterMotherTracks(RecoTrack const* const recoTrack)
{
  // track is fitted
  if (!recoTrack->hasTrackFitStatus() || !recoTrack->wasFitSuccessful()) return false;

  // last point of the track is > m_precutRho and > m_precutZ from outer walls of CDC
  ROOT::Math::XYZVector posLast(recoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  if (!ifInCDC(posLast) && recoTrack->getNumberOfCDCHits()) return false;

  // track is not curled back to IP
  // The first layer of CDC has 16.8 cm radius; thus, the track which ends inside 17 cm cylinder is either short or curler.
  // The requirement of >= 10 hits ensures that it is a curler (obtained empirically)
  int nCDCHits = recoTrack->getNumberOfCDCHits();
  if (nCDCHits >= 10 && posLast.Perp2() < 289) return false;

  // first point of the track is inside inner layers of VXD
  // The mother candidate should start inside VXD. The radius of outer two SVD layers are 10.4 and 13.5 cm,
  // so we set 12 cm cut (between these layers).
  ROOT::Math::XYZVector posFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if (posFirst.Perp2() > 144) return false;

  // impact parameter in rphi < 1 cm
  // conservative enough assuming possible bad resolution of mother track due to daughter hits assignment
  ROOT::Math::XYZVector momFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
  const double Bz = BFieldManager::getFieldInTesla({momFirst.X(),
                                                    momFirst.Y(),
                                                    momFirst.Z()}).Z();
  Helix helixIP(posFirst, momFirst, static_cast<short>(recoTrack->getTrackFitStatus()->getCharge()), Bz);
  if (fabs(helixIP.getD0()) > 1) return false;
  return true;
}

/// Check if the track can be a daughter candidate based on some simple selections.
/// They are conservative enough to reduces the combinatorial background and save as much signal events as possible.
bool KinkFinderModule::preFilterDaughterTracks(RecoTrack const* const recoTrack)
{
  // track is fitted
  if (!recoTrack->hasTrackFitStatus() || !recoTrack->wasFitSuccessful()) return false;

  // first and last points of the track are > m_precutRho and > m_precutZ from outer walls of CDC (against back splashes)
  ROOT::Math::XYZVector posLast(recoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  ROOT::Math::XYZVector posFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if (!ifInCDC(posLast) && !ifInCDC(posFirst)) return false;

  // first point of the track is outside inner layers of VXD, or impact parameter in rphi > 1 cm
  // inverse criteria for mother
  ROOT::Math::XYZVector momFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
  const double Bz = BFieldManager::getFieldInTesla({momFirst.X(),
                                                    momFirst.Y(),
                                                    momFirst.Z()}).Z();
  Helix helixIP(posFirst, momFirst, static_cast<short>(recoTrack->getTrackFitStatus()->getCharge()), Bz);
  if (posFirst.Perp2() < 144 && fabs(helixIP.getD0()) < 1) return false;
  return true;
}

/// Check if the track is a candidate for splitting based on some simple selections.
/// They are optimized to reduce the splitting of ordinary (non-decayed) tracks.
bool KinkFinderModule::preFilterTracksToSplit(RecoTrack const* const recoTrack, Track const* const track)
{
  // track is fitted
  if (!recoTrack->hasTrackFitStatus() || !recoTrack->wasFitSuccessful()) return false;
  if (recoTrack->getTrackFitStatus()->getPVal() > m_precutSplitPValue) return false;
  int nCDCHits = track->getTrackFitResultWithClosestMass(Const::pion)->getHitPatternCDC().getNHits();
  if (nCDCHits > m_precutSplitNCDCHit) return false;
  // more than 5 CDC hits to have enough hits for possible splitting
  if (nCDCHits < 5) return false;

  // first point of the track is close to the inner wall of the CDC
  // we set 18 cm cut to have both tracks with and without VXD hits
  ROOT::Math::XYZVector posFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if (posFirst.Perp2() > 324) return false;

  // impact parameter in rphi < 2 cm
  // Loose enough assuming possible bad resolution of track due to daughter hits assignment
  ROOT::Math::XYZVector momFirst(recoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
  const double Bz = BFieldManager::getFieldInTesla({momFirst.X(),
                                                    momFirst.Y(),
                                                    momFirst.Z()}).Z();
  Helix helixIP(posFirst, momFirst, static_cast<short>(recoTrack->getTrackFitStatus()->getCharge()), Bz);
  if (fabs(helixIP.getD0()) > 2) return false;

  return true;
}

/// Track pair preselection based on distance between two tracks with six different options.
short KinkFinderModule::isTrackPairSelected(const Track* motherTrack, const Track* daughterTrack)
{
  constexpr double M_PI_over_6 = M_PI / 6.;
  constexpr double cos_M_PI_over_6 = cos(M_PI_over_6);

  // get recoTracks and check that they are not the same
  RecoTrack const* const motherRecoTrack = motherTrack->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  RecoTrack const* const daughterRecoTrack = daughterTrack->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  if (motherRecoTrack == daughterRecoTrack) return 0;

  // Filter 1 check.
  ROOT::Math::XYZVector motherPosLast(motherRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  ROOT::Math::XYZVector daughterPosFirst(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
  if ((daughterPosFirst - motherPosLast).Mag2() < m_precutDistanceSquared) return 1;

  // Filter 2 check.
  ROOT::Math::XYZVector daughterPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
  if ((daughterPosLast - motherPosLast).Mag2() < m_precutDistanceSquared) return 2;

  // Filter 3 check. Here the same direction of daughter and mother is checked to exclude intersection of tracks
  // from different hemispheres.
  // Here and further M_PI_over_6 criteria was obtained empirically: it conservatively reduces the combinatorial
  // background while saving as much signal events as possible.
  if (daughterPosLast.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      daughterPosFirst.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      fabs(daughterPosLast.Phi() - motherPosLast.Phi()) < M_PI_over_6 ||
      fabs(daughterPosFirst.Phi() - motherPosLast.Phi()) < M_PI_over_6) {

    ROOT::Math::XYZVector daughterPosClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
    ROOT::Math::XYZVector daughterMomClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
    const double Bz = BFieldManager::getFieldInTesla({daughterPosClosestToMotherPosLast.X(),
                                                      daughterPosClosestToMotherPosLast.Y(),
                                                      daughterPosClosestToMotherPosLast.Z()}).Z();
    // daughter Helix with move to mother last point
    Helix daughterHelixClosestToMotherPosLast(daughterPosClosestToMotherPosLast,
                                              daughterMomClosestToMotherPosLast,
                                              static_cast<short>(daughterRecoTrack->getTrackFitStatus()->getCharge()),
                                              Bz);
    daughterHelixClosestToMotherPosLast.passiveMoveBy(motherPosLast);

    if ((daughterHelixClosestToMotherPosLast.getD0() * daughterHelixClosestToMotherPosLast.getD0() +
         daughterHelixClosestToMotherPosLast.getZ0() * daughterHelixClosestToMotherPosLast.getZ0()) <
        m_precutDistanceSquared)
      return 3;
  }

  // Filter 4 check.
  if ((daughterPosFirst - motherPosLast).Perp2() < m_precutDistance2DSquared)
    return 4;

  // Filter 5 check.
  if ((daughterPosLast - motherPosLast).Perp2() < m_precutDistance2DSquared)
    return 5;

  // Filter 6 check. sHere the same direction of daughter and mother is checked to exclude intersection of tracks
  // from different hemispheres.
  if (daughterPosLast.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      daughterPosFirst.Unit().Dot(motherPosLast.Unit()) > cos_M_PI_over_6 ||
      fabs(daughterPosLast.Phi() - motherPosLast.Phi()) < M_PI_over_6 ||
      fabs(daughterPosFirst.Phi() - motherPosLast.Phi()) < M_PI_over_6) {

    ROOT::Math::XYZVector daughterPosClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
    ROOT::Math::XYZVector daughterMomClosestToMotherPosLast(daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
    const double Bz = BFieldManager::getFieldInTesla({daughterPosClosestToMotherPosLast.X(),
                                                      daughterPosClosestToMotherPosLast.Y(),
                                                      daughterPosClosestToMotherPosLast.Z()}).Z();
    // daughter Helix with move to mother last point
    Helix daughterHelixClosestToMotherPosLast(daughterPosClosestToMotherPosLast,
                                              daughterMomClosestToMotherPosLast,
                                              static_cast<short>(daughterRecoTrack->getTrackFitStatus()->getCharge()),
                                              Bz);
    daughterHelixClosestToMotherPosLast.passiveMoveBy(motherPosLast);

    if (fabs(daughterHelixClosestToMotherPosLast.getD0()) < m_precutDistance2D)
      return 6;
  }

  // No filter is passed.
  return 0;

}

/// Kink fitting and storing.
void KinkFinderModule::fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag)
{
  bool ok = m_kinkFitter->fitAndStore(trackMother, trackDaughter, filterFlag);
  if (ok) {
    ++m_allStored;
    switch (filterFlag) {
      case 1:
        ++m_f1Stored;
        break;
      case 2:
        ++m_f1Stored;
        break;
      case 3:
        ++m_f2Stored;
        break;
      case 4:
        ++m_f1Stored;
        break;
      case 5:
        ++m_f1Stored;
        break;
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
