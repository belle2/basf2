/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <CLHEP/Vector/LorentzVector.h>
#include "CLHEP/Geometry/Point3D.h"


// Std lib includes
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <tuple>

/* Belle2 headers. */
#include <eklm/modules/EKLMTrackMatchCollector/EKLMTrackMatchCollectorModule.h>
#include <eklm/modules/EKLMTrackEff/EKLMTrackEffModule.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMAlignmentHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>



#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

REG_MODULE(EKLMTrackMatchCollector)

EKLMTrackMatchCollectorModule::EKLMTrackMatchCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for KLM channel status calibration (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);
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
  m_hitAlign.isRequired();
  m_recoTracks.registerRelationTo(m_hit2ds);
  m_GeoDat = &(EKLM::GeometryData::Instance());

  m_MatchedDigitsInPlane = new TH1F("Matched Digits in planeNumber", "", 208, 1, 208);
  m_AllExtHitsInPlane = new TH1F("All ExtHits in planeNumber", "", 208, 1, 208);

  registerObject<TH1F>("MatchedDigitsInPlane", m_MatchedDigitsInPlane);
  registerObject<TH1F>("AllExtHitsInPlane", m_AllExtHitsInPlane);
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

  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  // Get all info about part of detector
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);
  return std::make_tuple(copyid, idEndcap, idLayer, idSector, idPlane, idStrip);
}

double EKLMTrackMatchCollectorModule::getMinDist(const ExtHit& ext_hit) const
{
  double distance_value = 0;
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);

  for (auto hit_2_d : m_hit2ds) {
    // Asking dor a match sector, layer etc.
    if (hit_2_d.getLayer() == idLayer && hit_2_d.getEndcap() == idEndcap && hit_2_d.getSector() == idSector) {
      TVector3 hit2d_pos = hit_2_d.getPosition();
      TVector3 distance_vector = hit2d_pos - ext_hit.getPosition();
      if (distance_value != 0) {
        if (distance_vector.Mag() < distance_value) distance_value = distance_vector.Mag();
      } else {
        distance_value = distance_vector.Mag();
      }
    }
  }
  if (distance_value == 0) return -999; //If no match return -999
  return distance_value;
}

double EKLMTrackMatchCollectorModule::getMinDist1d(const ExtHit& ext_hit) const
{
  double distance_value = -1;
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);

  for (auto hit_1d : m_digits) {
    // Asking dor a match sector, layer etc.
    if (hit_1d.getLayer() == idLayer && hit_1d.getEndcap() == idEndcap
        && hit_1d.getSector() == idSector && hit_1d.getPlane() == idPlane) {
      double hit_1d_pos = hit_1d.getStrip();
      double distance_vector = std::fabs(hit_1d_pos - idStrip);
      if (distance_value != -1) {
        if (distance_vector < distance_value) distance_value = distance_vector;
      } else {
        distance_value = distance_vector;
      }
    }
  }
  if (distance_value == -1) return -999; //If no match return -999
  return distance_value;
}

std::pair<bool, bool> EKLMTrackMatchCollectorModule::trackCheck(int number_of_required_hits) const
{
  // Map to calc number of hits
  // Forward - 2, backward - 1
  bool mark_forward = false;
  bool mark_backward = false;

  std::map<int, int> endcap_hits = { {1, 0}, {2, 0} };
  for (auto hit_ : m_hit2ds) {
    endcap_hits[hit_.getEndcap()]++;
  }

  if (endcap_hits[1] > number_of_required_hits) mark_forward = true;
  if (endcap_hits[2] > number_of_required_hits) mark_backward = true;

  return std::make_pair(mark_forward, mark_backward);
}

bool EKLMTrackMatchCollectorModule::digitsMatching(const ExtHit& ext_hit, double allowed_distance) const
{
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);
  TVector3 ext_hit_pos = ext_hit.getPosition();

  for (auto hit_1d : m_digits) {
    TVector3 hit1d_pos = hit_1d.getPosition();
    TVector3 distance = hit1d_pos - ext_hit_pos;

    if (hit_1d.getLayer() == idLayer && hit_1d.getEndcap() == idEndcap
        && hit_1d.getSector() == idSector && hit_1d.getPlane() == idPlane
        && (idStrip > hit_1d.getStrip() - allowed_distance && idStrip < hit_1d.getStrip() + allowed_distance))
    { return true; }
  }

  return false;
}

bool EKLMTrackMatchCollectorModule::hit2dsMatching(const ExtHit& ext_hit, double allowed_distance) const
{
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);
  TVector3 ext_hit_pos = ext_hit.getPosition();

  for (auto hit_2d : m_hit2ds) {
    TVector3 hit1d_pos = hit_2d.getPosition();
    TVector3 distance = hit1d_pos - ext_hit_pos;

    if (hit_2d.getLayer() == idLayer && hit_2d.getEndcap() == idEndcap
        && hit_2d.getSector() == idSector && distance.Mag() < allowed_distance) { return true; }
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

double EKLMTrackMatchCollectorModule::angleThetaBetweenTracks(const StoreArray<Track>& selected_tracks) const
{
  TVector3 first_trk;
  TVector3 second_trk;
  bool trk_cnt = false;

  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    if (trk_cnt) first_trk = fitResult->getMomentum();
    else second_trk = fitResult->getMomentum();
    trk_cnt = true;
  }

  TVector3 result = first_trk - second_trk;
  return result.Theta();
}

void EKLMTrackMatchCollectorModule::extHitsCorr(const StoreArray<Track>& selected_tracks)
{
  int first_trk = 0;
  int second_trk = 0;
  bool trk_cnt = false;

  for (const Track& trk : selected_tracks) {
    for (const auto& ext_hit : trk.getRelationsTo<ExtHit>()) {
      if (ext_hit.getDetectorID() != Const::EDetector::EKLM) continue;
      if (ext_hit.getStatus() != EXT_ENTER) continue;
      int copyid = ext_hit.getCopyID();
      if (copyid == -1) continue;
      if (!trk_cnt) first_trk++;
      if (trk_cnt) second_trk++;
    }
    trk_cnt = true;
  }

  m_ExtHitsCorrelation->Fill(first_trk, second_trk);
}

void EKLMTrackMatchCollectorModule::hit2dsCorr()
{
  int bwd_hits = 0;
  int fwd_hits = 0;
  for (auto hit : m_hit2ds) {
    if (hit.getEndcap() == 1) bwd_hits++;
    if (hit.getEndcap() == 2) fwd_hits++;
  }
  m_Hit2dsCorrelation->Fill(fwd_hits, bwd_hits);
}

void EKLMTrackMatchCollectorModule::thetaCorr(const StoreArray<Track>& selected_tracks)
{
  double first_trk = 0;
  double second_trk = 0;
  bool trk_cnt = false;

  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    if (!trk_cnt) first_trk = fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi;
    if (trk_cnt) second_trk = fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi;
    trk_cnt = true;
  }
  m_ThetaCorrelationHist->Fill(first_trk, second_trk);
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

      for (const auto& ext_hit : track.getRelationsTo<ExtHit>()) {

        auto [copyid, idEndcap, idLayer, idSector, idPlane, idStrip] = checkExtHit(ext_hit);
        if (copyid == -1) continue;

        TVector3 ext_hit_pos = ext_hit.getPosition();

        int planeNum = m_ElementNumbers->planeNumber(idEndcap, idLayer, idSector, idPlane);

        if (idEndcap == 2 && mark_forward) {
          if (hit2dsMatching(ext_hit, m_AllowedDistance2D)) {
          }

          if (digitsMatching(ext_hit, m_AllowedDistance1D)) {
            m_MatchedDigitsInPlane->Fill(planeNum);
          }

          m_AllExtHitsInPlane->Fill(planeNum);

        } else if (idEndcap == 1 && mark_backward) {
          if (hit2dsMatching(ext_hit, m_AllowedDistance2D)) {
          }


          if (digitsMatching(ext_hit, m_AllowedDistance1D)) {
            m_MatchedDigitsInPlane->Fill(planeNum);
          }

          m_AllExtHitsInPlane->Fill(planeNum);
        }
      }
    }
  }

}

void EKLMTrackMatchCollectorModule::closeRun()
{

}