/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "CLHEP/Geometry/Point3D.h"

// Std lib includes
#include <map>
#include <string>
#include <tuple>

/* ROOT headers. */
#include <TH1F.h>

/* Belle2 headers. */
#include <klm/eklm/modules/EKLMTrackMatchCollector/EKLMTrackMatchCollectorModule.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>

using namespace Belle2;

REG_MODULE(EKLMTrackMatchCollector)

EKLMTrackMatchCollectorModule::EKLMTrackMatchCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for EKLM strips efficiency (data collection).");
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Max distance in strips number to 1D hit from extHit to be still matched (default 8 strips)", double(8));
  addParam("z0_d0", m_D0Z0, "D0_z0 distance", double(5));
  setPropertyFlags(c_ParallelProcessingCertified);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_GeoDat = nullptr;
}

EKLMTrackMatchCollectorModule::~EKLMTrackMatchCollectorModule()
{
}

void EKLMTrackMatchCollectorModule::prepare()
{

  m_digits.isRequired();
  m_hit2ds.isRequired();
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_extHits.isRequired();
  m_recoTracks.registerRelationTo(m_hit2ds);
  m_GeoDat = &(EKLM::GeometryData::Instance());

  TH1F* MatchedDigitsInPlane =
    new TH1F("Matched Digits in planeNumber", "",
             EKLMElementNumbers::getMaximalPlaneGlobalNumber(),
             0.5, EKLMElementNumbers::getMaximalPlaneGlobalNumber() + 0.5);
  TH1F* AllExtHitsInPlane = new TH1F(
    "All ExtHits in planeNumber", "",
    EKLMElementNumbers::getMaximalPlaneGlobalNumber(),
    0.5, EKLMElementNumbers::getMaximalPlaneGlobalNumber() + 0.5);

  registerObject<TH1F>("MatchedDigitsInPlane", MatchedDigitsInPlane);
  registerObject<TH1F>("AllExtHitsInPlane", AllExtHitsInPlane);
}

std::tuple<int, int, int, int, int, int> EKLMTrackMatchCollectorModule::checkExtHit(const ExtHit& ext_hit) const
{
  // If ExtHit NOT in EKLM continue
  if (ext_hit.getDetectorID() != Const::EDetector::EKLM) return std::make_tuple(-1, -1, -1, -1, -1, -1);
  // Choose extHits that just enter in sensetive volume
  if (ext_hit.getStatus() != EXT_ENTER) return std::make_tuple(-1, -1, -1, -1, -1, -1);
  // Get subdetector component ID
  int copyid = ext_hit.getCopyID();
  if (copyid < 1 || copyid > 15600) return std::make_tuple(-1, -1, -1, -1, -1, -1);

  int idSection = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  // Get all info about part of detector
  m_GeoDat->stripNumberToElementNumbers(copyid, &idSection, &idLayer, &idSector, &idPlane, &idStrip);
  return std::make_tuple(copyid, idSection, idLayer, idSector, idPlane, idStrip);
}

std::pair<bool, bool> EKLMTrackMatchCollectorModule::trackCheck(int number_of_required_hits) const
{
  // Map to calc number of hits
  // Forward - 2, backward - 1
  bool mark_forward = false;
  bool mark_backward = false;

  std::map<int, int> section_hits = { {1, 0}, {2, 0} };
  for (auto hit_ : m_hit2ds) {
    section_hits[hit_.getSection()]++;
  }

  if (section_hits[1] > number_of_required_hits) mark_forward = true;
  if (section_hits[2] > number_of_required_hits) mark_backward = true;

  return std::make_pair(mark_forward, mark_backward);
}

bool EKLMTrackMatchCollectorModule::digitsMatching(const ExtHit& ext_hit, double allowed_distance) const
{
  int copyid = ext_hit.getCopyID();
  int idSection = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idSection, &idLayer, &idSector, &idPlane, &idStrip);
  TVector3 ext_hit_pos = ext_hit.getPosition();

  for (auto hit_1d : m_digits) {
    TVector3 hit1d_pos = hit_1d.getPosition();
    TVector3 distance = hit1d_pos - ext_hit_pos;

    if (hit_1d.getLayer() == idLayer && hit_1d.getSection() == idSection
        && hit_1d.getSector() == idSector && hit_1d.getPlane() == idPlane
        && (idStrip > hit_1d.getStrip() - allowed_distance && idStrip < hit_1d.getStrip() + allowed_distance))
    { return true; }
  }

  return false;
}

double EKLMTrackMatchCollectorModule::getSumTrackEnergy(const StoreArray<Track>& selected_tracks) const
{
  double eneregy = 0;
  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    eneregy += fitResult->getEnergy();
  }
  return eneregy;
}


bool EKLMTrackMatchCollectorModule::d0z0Cut(const StoreArray<Track>& selected_tracks, double dist) const
{
  bool cosmic = false;

  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    double d0 = fitResult->getD0();
    double z0 = fitResult->getZ0();
    if (d0 > dist || z0 > dist) cosmic = true;
  }

  return cosmic;
}

bool EKLMTrackMatchCollectorModule::thetaAcceptance(const StoreArray<Track>& selected_tracks) const
{
  bool disacceptance = false;

  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    double theta = fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi;
    if (theta > 140 || theta < 20) disacceptance = true;
  }
  return disacceptance;
}


void EKLMTrackMatchCollectorModule::collect()
{
// Variables to mark possibility of using track for efficiency study
  auto [mark_forward, mark_backward] = trackCheck(5);

  for (const Track& track : m_tracks) {

    // Making different cuts
    const RecoTrack* recoTrack = track.getRelated<RecoTrack>();

    if (recoTrack && m_tracks.getEntries() == 2) {
      const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::muon);
      if (fitResult->getMomentum().Mag() > 11. || fitResult->getMomentum().Mag() < 1.) continue;
      // if (m_EnergyCut) {
      //   if (getSumTrackEnergy(m_tracks) > 10) continue;
      // }

      if (d0z0Cut(m_tracks, m_D0Z0)) continue;
      // if (thetaAcceptance(m_tracks)) continue;

      TH1F* MatchedDigitsInPlane;
      MatchedDigitsInPlane = getObjectPtr<TH1F>("MatchedDigitsInPlane");
      TH1F* AllExtHitsInPlane;
      AllExtHitsInPlane = getObjectPtr<TH1F>("AllExtHitsInPlane");

      for (const auto& ext_hit : track.getRelationsTo<ExtHit>()) {

        auto [copyid, idSection, idLayer, idSector, idPlane, idStrip] = checkExtHit(ext_hit);
        if (copyid == -1) continue;

        TVector3 ext_hit_pos = ext_hit.getPosition();

        int planeNum = m_ElementNumbers->planeNumber(idSection, idLayer, idSector, idPlane);

        if (idSection == 2 && mark_forward) {
          if (digitsMatching(ext_hit, m_AllowedDistance1D)) {
            MatchedDigitsInPlane->Fill(planeNum);
          }
          AllExtHitsInPlane->Fill(planeNum);

        } else if (idSection == 1 && mark_backward) {
          if (digitsMatching(ext_hit, m_AllowedDistance1D)) {
            MatchedDigitsInPlane->Fill(planeNum);
          }
          AllExtHitsInPlane->Fill(planeNum);
        }
      }
    }
  }

}

void EKLMTrackMatchCollectorModule::closeRun()
{
}
