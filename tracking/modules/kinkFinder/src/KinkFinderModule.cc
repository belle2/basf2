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
#include <framework/core/ModuleParam.templateDetails.h> // needed for complicated parameter types
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

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
                 "A loose cut on the pair vertex is applied before the fit (not considering material effects); "
                 "then, a vertex fit is performed, and only pairs passing a chi^2 (``vertexChi2Cut``) "
                 "and distance (``m_vertexDistanceCut``) "
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

  // mode of the kinkFitter
  addParam("kinkFitterMode", m_kinkFitterMode,
           "kinkFitter fitting mode "
           "0: find vertex only; 1: reassign hits; 2: flip and refit filter 2 tracks; 3: 1 + 2", 1);

  // preselection cuts
  addParam("precutRho", m_precutRho,
           "Preselection cut on transverse shift from the outer CDC wall for the track ending points. ", 10.);
  addParam("precutZ", m_precutZ, "Preselection cut on z shift from the outer CDC wall for the track ending points. ",
           0.);
  addParam("precutDistance", m_precutDistance, "Preselection cut on distance between ending points of two tracks. ",
           10.);
  addParam("precutDistance2D", m_precutDistance2D,
           "Preselection cut on 2D distance between ending points of two tracks (for bad z cases). ", 5.);

  // final cuts used in kinkFitter to decide if the kink candidate should be stored
  addParam("vertexChi2Cut", m_vertexChi2Cut,
           "Cut on Chi2 for the Kink vertex. ", 10000.);
  addParam("vertexDistanceCut", m_vertexDistanceCut,
           "Cut on distance between tracks at the Kink vertex. ", 1.);

}


void KinkFinderModule::initialize()
{
  m_tracks.isRequired(m_arrayNameTrack);
  StoreArray<RecoTrack> recoTracks(m_arrayNameRecoTrack);
  m_tracks.requireRelationTo(recoTracks);
  //All the other required StoreArrays are checked in the Construtor of the kinkFitter.


  m_kinkFitter = std::make_unique<kinkFitter>(m_arrayNameTFResult, m_arrayNameKink,
                                              m_arrayNameRecoTrack,
                                              m_arrayNameCopiedRecoTrack);
  m_kinkFitter->initializeCuts(m_vertexDistanceCut, m_vertexChi2Cut);
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

    if (preFilterMotherTracks(recoTrack)) {
      tracksMother.push_back(&track);
    }
    if (preFilterDaughterTracks(recoTrack)) {
      tracksDaughter.push_back(&track);
    }
  }

  // Reject boring events.
  if (tracksMother.empty() || tracksDaughter.empty()) {
    B2INFO("No interesting track pairs. tracksMother " << tracksMother.size() << ", tracksDaughter "
           << tracksDaughter.size());
    return;
  }

  // Pair up each mother track with each daughter track.
  for (auto& trackMother : tracksMother) {
    for (auto& trackDaughter : tracksDaughter) {
      short filterFlag = isTrackPairSelected(trackMother, trackDaughter);
      if (!filterFlag) continue;
      B2INFO("Found kink candidate with filterFlag " << filterFlag);
      fitAndStore(trackMother, trackDaughter, filterFlag);
    }
  }

}


void KinkFinderModule::terminate()
{
  B2INFO("===KinkFinder summary===========================================================");
  B2INFO("In total " << m_allStored << " kinks saved.");
  B2INFO("Among them with filter1 " << m_f1Stored << ", f2 " << m_f2Stored << ", f3 " << m_f3Stored << ", f4 "
         << m_f4Stored);
}

/// Test if the point in space is inside CDC (approximate custom geometry) with respect to shifts from outer wall.
bool KinkFinderModule::ifInCDC(TVector3& pos, double shiftR = 0, double shiftZ = 0)
{

  double z = (pos.Z() > 0) ? (pos.Z() + shiftZ) : (pos.Z() - shiftZ);
  double r = pos.Perp() + shiftR;

  bool r_part = (r < 112) && (r > 17 + shiftR);
  bool z_part;
  if (z > 0) {
    z_part = (r > (tan(17. / 180 * M_PI) * z)) && (r > (4.97 * z - 670));
  } else {
    z_part = (r > (-tan(30. / 180 * M_PI) * z)) && (r > (-4.2 * z - 192.5));
  }
  return ((r_part && z_part) || (pos.Perp() < 17 && pos.Perp() > 15));
}

/// Check if the track can be a mother candidate based on some simple selections.
bool KinkFinderModule::preFilterMotherTracks(RecoTrack const* const recoTrack)
{
  // track is fitted
  if (!recoTrack->hasTrackFitStatus() || !recoTrack->wasFitSuccessful()) return false;

  // last point of the track is > m_precutRho and > m_precutZ from outer walls of CDC
  TVector3 posLast = recoTrack->getMeasuredStateOnPlaneFromLastHit().getPos();
  if (!ifInCDC(posLast, m_precutRho, m_precutZ) && recoTrack->getNumberOfCDCHits()) return false;

  // track is not curled back to IP
  int nCDCHits = recoTrack->getNumberOfCDCHits();
  if (nCDCHits >= 10 && posLast.Perp() < 17) return false;

  // first point of the track is inside inner layers of VXD
  TVector3 posFirst = recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos();
  if (posFirst.Perp() > 12) return false;

  // impact parameter in rphi < 1 cm
  TVector3 momFirst = recoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom();
  Helix helixIP(ROOT::Math::XYZVector(posFirst),
                ROOT::Math::XYZVector(momFirst),
                static_cast<short>(recoTrack->getTrackFitStatus()->getCharge()),
                1.5);
  if (fabs(helixIP.getD0()) > 1) return false;
  return true;
}

/// Check if the track can be a daughter candidate based on some simple selections.
bool KinkFinderModule::preFilterDaughterTracks(RecoTrack const* const recoTrack)
{
  // track is fitted
  if (!recoTrack->hasTrackFitStatus() || !recoTrack->wasFitSuccessful()) return false;

  // first and last points of the track are > m_precutRho and > m_precutZ from outer walls of CDC (against back splashes)
  TVector3 posLast = recoTrack->getMeasuredStateOnPlaneFromLastHit().getPos();
  TVector3 posFirst = recoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos();
  if (!ifInCDC(posLast, m_precutRho, m_precutZ) && !ifInCDC(posFirst, m_precutRho, m_precutZ)) return false;

  // first point of the track is outside inner layers of VXD, or impact parameter in rphi > 1 cm
  TVector3 momFirst = recoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom();
  Helix helixIP(ROOT::Math::XYZVector(posFirst),
                ROOT::Math::XYZVector(momFirst),
                static_cast<short>(recoTrack->getTrackFitStatus()->getCharge()),
                1.5);
  if (posFirst.Perp() < 12 && fabs(helixIP.getD0()) < 1) return false;
  return true;

}

/// Track pair preselection based on distance between two tracks with four different options.
short KinkFinderModule::isTrackPairSelected(const Track* motherTrack, const Track* daughterTrack)
{

  // get recoTracks and check that they are not the same
  RecoTrack const* const motherRecoTrack = motherTrack->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  RecoTrack const* const daughterRecoTrack = daughterTrack->getRelated<RecoTrack>(m_arrayNameRecoTrack);
  if (motherRecoTrack == daughterRecoTrack) return 0;

  // Filter 1 check.
  TVector3 motherPosLast = motherRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos();
  TVector3 daughterPosFirst = daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos();
  if ((daughterPosFirst - motherPosLast).Mag() < m_precutDistance) return 1;

  // Filter 2 check.
  TVector3 daughterPosLast = daughterRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos();
  if ((daughterPosLast - motherPosLast).Mag() < m_precutDistance) return 2;

  // Filter 3 check. Here the same direction of daughter and mother is checked to exclude intersection of tracks
  // from different hemispheres.
  if (daughterPosLast.Angle(motherPosLast) < M_PI / 6 || daughterPosFirst.Angle(motherPosLast) < M_PI / 6) {

    TVector3 daughterPosClosestToMotherPosLast = daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos();
    TVector3 daughterMomClosestToMotherPosLast = daughterRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom();
    const double Bz = BFieldManager::getFieldInTesla({daughterPosClosestToMotherPosLast.X(),
                                                      daughterPosClosestToMotherPosLast.Y(),
                                                      daughterPosClosestToMotherPosLast.Z()}).Z();
    // daughter Helix with move to mother last point
    Helix daughterHelixClosestToMotherPosLast(ROOT::Math::XYZVector(daughterPosClosestToMotherPosLast),
                                              ROOT::Math::XYZVector(daughterMomClosestToMotherPosLast),
                                              static_cast<short>(daughterRecoTrack->getTrackFitStatus()->getCharge()),
                                              Bz);
    daughterHelixClosestToMotherPosLast.passiveMoveBy(ROOT::Math::XYZVector(motherPosLast));

    if (sqrt(daughterHelixClosestToMotherPosLast.getD0() * daughterHelixClosestToMotherPosLast.getD0() +
             daughterHelixClosestToMotherPosLast.getZ0() * daughterHelixClosestToMotherPosLast.getZ0()) <
        m_precutDistance)
      return 3;
  }

  // Filter 4 check. TODO: try to extend to any place in CDC, not only near VXD
  if ((motherRecoTrack->getNumberOfCDCHits()) < 10 &&
      ((daughterPosFirst.XYvector() - motherPosLast.XYvector()).Mod() < m_precutDistance2D))
    return 4;

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
        ++m_f2Stored;
        break;
      case 3:
        ++m_f3Stored;
        break;
      case 4:
        ++m_f4Stored;
        break;
    }
  }
}
