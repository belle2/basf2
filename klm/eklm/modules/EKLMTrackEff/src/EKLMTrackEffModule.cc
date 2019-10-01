/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <framework/datastore/StoreArray.h>
#include <klm/eklm/modules/EKLMTrackEff/EKLMTrackEffModule.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <klm/eklm/dataobjects/EKLMAlignmentHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>

/* ROOT headers. */
#include <TGraphErrors.h>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>

/* C++ headers. */
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <tuple>

using namespace Belle2;

REG_MODULE(EKLMTrackEff)

EKLMTrackEffModule::EKLMTrackEffModule() : Module()
{
  setDescription("Get track matching efficiency for EKLM");
  addParam("filename", m_filename, "Output root filename", std::string("EKLMTrackMatch.root"));
  addParam("debug", m_debug, "Debug/analysis mode", bool(false));
  addParam("z0_d0", m_D0Z0, "D0_z0 distance", double(5));
  addParam("AllowedDistance2D", m_AllowedDistance2D, "Max distance to 2D hit from extHit to be still matched (default 15cm)",
           double(15));
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Max distance in strips number to 1D hit from extHit to be still matched (default 8 strips)", double(8));
  // addParam("energy_cut", m_EnergyCut, " 2 < E < 10 full energy", bool(false));

  m_file = nullptr;
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_GeoDat = nullptr;
  m_AllExtHitsInPlane = nullptr;
  m_D0Distribution = nullptr;
  m_DigitMinDist = nullptr;
  m_ExtHitZTheta = nullptr;
  m_ExtHitsCorrelation = nullptr;
  m_ExtHitsLayerDistribTracks = nullptr;
  m_ExtHitsLayerDistribution = nullptr;
  m_ExtHitsZDistribTracks = nullptr;
  m_ExtHitsZDistribution = nullptr;
  m_Hit2dMatchedDistrib = nullptr;
  m_Hit2dZDistrib = nullptr;
  m_Hit2dsCorrelation = nullptr;
  m_Hit2dsNum = nullptr;
  m_MatchedDigitsInPlane = nullptr;
  m_MinHitDist = nullptr;
  m_MuonsAngle = nullptr;
  m_MuonsEnergy = nullptr;
  m_MuonsTheta = nullptr;
  m_MuonsThetaWithoutCut = nullptr;
  m_ThetaCorrelationHist = nullptr;
  m_Z0Distribution = nullptr;
  m_extHitsNum = nullptr;
  m_planesEff = nullptr;
}

EKLMTrackEffModule::~EKLMTrackEffModule()
{
}

/* Define some functions in EKLMTrackEffModule namespace
  and then use them in EKLMTrackEffModule::event like in
  main() function. That helps support code readable and free to modify */

void EKLMTrackEffModule::initialize()
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

  // Set of declarated variables
  m_file = new TFile(m_filename.c_str(), "recreate");
  m_file->cd();

  m_MatchedDigitsInPlane = new TH1F("Matched Digits in planeNumber", "", 208, 1, 208);
  m_AllExtHitsInPlane = new TH1F("All ExtHits in planeNumber", "", 208, 1, 208);
  m_planesEff = new TH1F("Planes efficiency", "", 208, 1, 208);

  for (int i = 1; i < 5; ++i) {
    std::string name = "Matched 2D hits in layer sector " + std::to_string(i);
    m_MatchingByLayer[i] = new TH1F(name.data(), " ", 27, -12, 15);
    m_MatchingByLayer[i]->GetYaxis()->SetTitle("Num of matched 2Dhits");
    m_MatchingByLayer[i]->GetXaxis()->SetTitle("Layer");
  }

  for (int i = 1; i < 5; ++i) {
    std::string name = "All ExtHits in layer sector " + std::to_string(i);
    m_AllExtHitsInLayer[i] = new TH1F(name.data(), " ", 27, -12, 15);
    m_AllExtHitsInLayer[i]->GetYaxis()->SetTitle("Num of all exthits");
    m_AllExtHitsInLayer[i]->GetXaxis()->SetTitle("Layer");
  }

  for (int i = 1; i < 5; ++i) {
    std::string name = "Layer efficiency sector " + std::to_string(i);
    m_LayersEff[i] = new TH1F(name.data(), " ", 27, -12, 15);
    m_LayersEff[i]->GetYaxis()->SetTitle("Layer efficiency");
    m_LayersEff[i]->GetXaxis()->SetTitle("Layer");
  }

  for (int i = 1; i < 5; ++i) {
    std::string name = "Matched Digits in layer sector " + std::to_string(i);
    m_MatchingByPlaneVis[i] = new TH1F(name.data(), " ", 53, -24, 29);
    m_MatchingByPlaneVis[i]->GetYaxis()->SetTitle("Num of matched Digits");
    m_MatchingByPlaneVis[i]->GetXaxis()->SetTitle("2 * Layer + plane - 2");
  }

  for (int i = 1; i < 5; ++i) {
    std::string name = "All ExtHits in plane sector " + std::to_string(i);
    m_AllExtHitsInPlaneVis[i] = new TH1F(name.data(), " ", 53, -24, 29);
    m_AllExtHitsInPlaneVis[i]->GetYaxis()->SetTitle("Num of all exthits");
    m_AllExtHitsInPlaneVis[i]->GetXaxis()->SetTitle("2 * Layer + plane - 2");
  }

  for (int i = 1; i < 5; ++i) {
    std::string name = "Plane efficiency sector " + std::to_string(i);
    m_PlanesEffVis[i] = new TH1F(name.data(), " ", 53, -24, 29);
    m_PlanesEffVis[i]->GetYaxis()->SetTitle("Num of all exthits");
    m_PlanesEffVis[i]->GetXaxis()->SetTitle("2 * Layer + plane - 2");
  }


  //* For Debug / analysis *//

  m_ExtHitsZDistribution = new TH1F("z_distrib_without_tracks", "", 600, -300, 420);
  m_extHitsNum = new TH1F("Number_of_exthits", "", 59, 1, 60);
  m_Hit2dsNum = new TH1F("Number_of_Hit2ds", "", 59, 1, 60);

  m_ExtHitZTheta = new TH2F("z/theta distribution of extHits", "", 26, -12, 14, 90, 0, 180);
  m_ExtHitZTheta->GetXaxis()->SetTitle("Layer");
  m_ExtHitZTheta->GetYaxis()->SetTitle("Theta");

  for (int i = 1; i < 5; ++i) {
    std::string name = "Sector " + std::to_string(i);
    m_ExtHitsSectorHists[i] = new TH1F(name.data(), " ", 40, -20, 20);
  }

  m_ExtHitsZDistribTracks = new TH1F("z_distrib_with_tracks", "", 600, -300, 420);

  for (int i = 1; i < 5; ++i) {
    std::string name = "With track by Sector " + std::to_string(i);
    m_ExtHitsSectorHistsTrack[i] = new TH1F(name.data(), " ", 40, -20, 20);
  }


  m_Hit2dZDistrib = new TH1F("Hit2d_z_distib", "", 600, -300, 420);

  for (int i = 1; i < 5; ++i) {
    std::string name = "Hit2D with Sector " + std::to_string(i);
    m_Hit2dSectorHists[i] = new TH1F(name.data(), " ", 40, -20, 20);
  }

  m_MinHitDist = new TH1F("min_dist", "", 50, 0, 50);
  m_DigitMinDist = new TH1F("min dist for 1d", "", 20, 0, 20);

  m_ExtHitsLayerDistribution = new TH1F("ext_hit_layer_distrib", "", 26, -12, 14);

  m_Hit2dMatchedDistrib = new TH1F("Hit2ds_matched_distrib", "", 26, -12, 14);

  m_ExtHitsLayerDistribTracks = new TH1F("Ext_hits_layer_distrib_track", "", 26, -12, 14);

  m_ExtHitsCorrelation = new TH2F("Corr_between_hits_in_tracks", "", 34, 1, 35, 34, 1, 35);

  m_Hit2dsCorrelation = new TH2F("Corr_between_hit2ds_int_back_and_fwd", "", 14, 1, 15, 14, 1, 15);
  m_Hit2dsCorrelation->GetYaxis()->SetTitle("Bwd");
  m_Hit2dsCorrelation->GetXaxis()->SetTitle("Fwd");

  m_ThetaCorrelationHist = new TH2F("Theta_corr", "", 45, 0, 180, 45, 0, 180);
  m_MuonsEnergy = new TH1F("Muons energy", "", 100, 1, 12);
  m_MuonsTheta = new TH1F("Muons theta", "", 45, 0, 180);
  m_MuonsThetaWithoutCut = new TH1F("Muons theta witout cut", "", 45, 0, 180);
  m_MuonsAngle = new TH1F("Angle between muons", "", 30, 0, CLHEP::pi);

  m_D0Distribution = new TH1F("D0", "", 120, 0, 6);
  m_Z0Distribution = new TH1F("Z0", "", 120, 0, 6);
}

std::tuple<int, int, int, int, int, int> EKLMTrackEffModule::checkExtHit(const ExtHit& ext_hit) const
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

double EKLMTrackEffModule::getMinDist(const ExtHit& ext_hit) const
{
  double distance_value = 0;
  int copyid = ext_hit.getCopyID();
  int idSection = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idSection, &idLayer, &idSector, &idPlane, &idStrip);

  for (auto hit_2_d : m_hit2ds) {
    // Asking dor a match sector, layer etc.
    if (hit_2_d.getLayer() == idLayer && hit_2_d.getSection() == idSection && hit_2_d.getSector() == idSector) {
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

double EKLMTrackEffModule::getMinDist1d(const ExtHit& ext_hit) const
{
  double distance_value = -1;
  int copyid = ext_hit.getCopyID();
  int idSection = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idSection, &idLayer, &idSector, &idPlane, &idStrip);

  for (auto hit_1d : m_digits) {
    // Asking dor a match sector, layer etc.
    if (hit_1d.getLayer() == idLayer && hit_1d.getSection() == idSection
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

std::pair<bool, bool> EKLMTrackEffModule::trackCheck(int number_of_required_hits) const
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

bool EKLMTrackEffModule::digitsMatching(const ExtHit& ext_hit, double allowed_distance) const
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

bool EKLMTrackEffModule::hit2dsMatching(const ExtHit& ext_hit, double allowed_distance) const
{
  int copyid = ext_hit.getCopyID();
  int idSection = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idSection, &idLayer, &idSector, &idPlane, &idStrip);
  TVector3 ext_hit_pos = ext_hit.getPosition();

  for (auto hit_2d : m_hit2ds) {
    TVector3 hit1d_pos = hit_2d.getPosition();
    TVector3 distance = hit1d_pos - ext_hit_pos;

    if (hit_2d.getLayer() == idLayer && hit_2d.getSection() == idSection
        && hit_2d.getSector() == idSector && distance.Mag() < allowed_distance) { return true; }
  }

  return false;
}

double EKLMTrackEffModule::getSumTrackEnergy(const StoreArray<Track>& selected_tracks) const
{
  double eneregy = 0;
  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    eneregy += fitResult->getEnergy();
  }
  return eneregy;
}

double EKLMTrackEffModule::angleThetaBetweenTracks(const StoreArray<Track>& selected_tracks) const
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

void EKLMTrackEffModule::extHitsCorr(const StoreArray<Track>& selected_tracks)
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

void EKLMTrackEffModule::hit2dsCorr()
{
  int bwd_hits = 0;
  int fwd_hits = 0;
  for (auto hit : m_hit2ds) {
    if (hit.getSection() == 1) bwd_hits++;
    if (hit.getSection() == 2) fwd_hits++;
  }
  m_Hit2dsCorrelation->Fill(fwd_hits, bwd_hits);
}

void EKLMTrackEffModule::thetaCorr(const StoreArray<Track>& selected_tracks)
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

bool EKLMTrackEffModule::d0z0Cut(const StoreArray<Track>& selected_tracks, double dist) const
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

bool EKLMTrackEffModule::thetaAcceptance(const StoreArray<Track>& selected_tracks) const
{
  bool disacceptance = false;

  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    double theta = fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi;
    if (theta > 140 || theta < 20) disacceptance = true;
  }
  return disacceptance;
}


void EKLMTrackEffModule::event()
{

  /********************************
    ExtHits with track z distrib
     Layer Hit2ds and Digits eff
  **********************************/

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

        auto [copyid, idSection, idLayer, idSector, idPlane, idStrip] = checkExtHit(ext_hit);
        if (copyid == -1) continue;

        TVector3 ext_hit_pos = ext_hit.getPosition();

        int planeNum = m_ElementNumbers->planeNumber(idSection, idLayer, idSector, idPlane);

        if (idSection == 2 && mark_forward) {
          if (hit2dsMatching(ext_hit, m_AllowedDistance2D)) {
            m_MatchingByLayer[idSector]->Fill(idLayer);
            m_Hit2dMatchedDistrib->Fill(idLayer);
          }

          m_AllExtHitsInLayer[idSector]->Fill(idLayer);

          if (digitsMatching(ext_hit, m_AllowedDistance1D)) {
            m_MatchedDigitsInPlane->Fill(planeNum);
            m_MatchingByPlaneVis[idSector]->Fill(2 * idLayer + idPlane - 2);
          }

          m_AllExtHitsInPlane->Fill(planeNum);
          m_AllExtHitsInPlaneVis[idSector]->Fill(2 * idLayer + idPlane - 2);

        } else if (idSection == 1 && mark_backward) {
          if (hit2dsMatching(ext_hit, m_AllowedDistance2D)) {
            m_MatchingByLayer[idSector]->Fill(-idLayer);
            m_Hit2dMatchedDistrib->Fill(-idLayer);
          }

          m_AllExtHitsInLayer[idSector]->Fill(-idLayer);

          if (digitsMatching(ext_hit, m_AllowedDistance1D)) {
            m_MatchedDigitsInPlane->Fill(planeNum);
            m_MatchingByPlaneVis[idSector]->Fill(-2 * idLayer - idPlane + 2);
          }

          m_AllExtHitsInPlane->Fill(planeNum);
          m_AllExtHitsInPlaneVis[idSector]->Fill(-2 * idLayer - idPlane + 2);
        }
      }
    }
  }

  //* DEBUG MODE ADDITIONAL CODE START *//
  if (m_debug) {

    // Track counter variable to make possible know
    // When is the secons track of the loop
    // To use in correlations
    int trk_cnt = 0;

    for (const Track& track : m_tracks) {
      trk_cnt++;

      // Making different cuts
      const RecoTrack* recoTrack = track.getRelated<RecoTrack>();

      if (recoTrack && m_tracks.getEntries() == 2) {
        const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::muon);
        if (fitResult->getMomentum().Mag() > 11. || fitResult->getMomentum().Mag() < 1.) continue;
        m_MuonsThetaWithoutCut->Fill(fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi);
        // if (m_EnergyCut) {
        // if (getSumTrackEnergy(m_tracks) > 10) continue;
        // }
        double d0 = fitResult->getD0();
        double z0 = fitResult->getZ0();
        m_D0Distribution->Fill(d0);
        m_Z0Distribution->Fill(z0);
        if (d0z0Cut(m_tracks, m_D0Z0)) continue;
        // if (thetaAcceptance(m_tracks)) continue;
        m_MuonsTheta->Fill(fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi);

        // Only in debug mode
        // Searching for correlations
        // Use this functions only when second track is OK
        if (trk_cnt == 2) {
          extHitsCorr(m_tracks);
          hit2dsCorr();
          thetaCorr(m_tracks);
          //Get tracks energy distib
          m_MuonsEnergy->Fill(getSumTrackEnergy(m_tracks));
          m_MuonsAngle->Fill(angleThetaBetweenTracks(m_tracks));
        }

        for (const auto& ext_hit : track.getRelationsTo<ExtHit>()) {

          auto [copyid, idSection, idLayer, idSector, idPlane, idStrip] = checkExtHit(ext_hit);
          if (copyid == -1) continue;

          TVector3 ext_hit_pos = ext_hit.getPosition();
          double theta = ext_hit_pos.Theta() * 180.0 / CLHEP::pi;
          // double theta = (ext_hit_pos.Phi() < CLHEP::pi / 2 && ext_hit_pos.Phi() > -CLHEP::pi / 2 ) ? ext_hit_pos.Theta() * 180.0 / CLHEP::pi : -ext_hit_pos.Theta() * 180.0 / CLHEP::pi;

          // Maybe only EKLM zone?
          // if (theta < 25 || theta > 37 ) {
          //   if (theta < 130 || theta > 145) break;
          // }

          // If Forward (2) else Backward(1)
          if (idSection == 2 && mark_forward) {
            m_ExtHitsSectorHistsTrack[idSector]->Fill(idLayer);
            m_ExtHitsLayerDistribTracks->Fill(idLayer);
            m_ExtHitsZDistribTracks->Fill(ext_hit.getPosition().z());
            // To get all exthits in event
            m_ExtHitZTheta->Fill(idLayer, theta);
          } else if (idSection == 1 && mark_backward) {
            m_ExtHitsSectorHistsTrack[idSector]->Fill(-idLayer);
            m_ExtHitsLayerDistribTracks->Fill(-idLayer);
            m_ExtHitsZDistribTracks->Fill(ext_hit.getPosition().z());
            m_ExtHitZTheta->Fill(-idLayer, theta);
          }

          // To get distance distribution
          m_MinHitDist->Fill(getMinDist(ext_hit));
          m_DigitMinDist->Fill(getMinDist1d(ext_hit));

          //* End of ExtHits loop *//
        }
        //* End of If *//
      }
      //* End of Tracks loop *//
    }

    // ExtHits counter
    int64_t counter = 0;

    /*All ExtHits and Hit2ds distributions */

    // Loop by all extHits
    for (auto ext_hit : m_extHits) {

      auto [copyid, idSection, idLayer, idSector, idPlane, idStrip] = checkExtHit(ext_hit);
      if (copyid == -1) continue;

      // If Forward (2) else Backward(1)
      if (idSection == 2) {
        m_ExtHitsLayerDistribution->Fill(idLayer);
        m_ExtHitsSectorHists[idSector]->Fill(idLayer);
      } else {
        m_ExtHitsLayerDistribution->Fill(-idLayer);
        m_ExtHitsSectorHists[idSector]->Fill(-idLayer);
      }

      ++counter;
      m_ExtHitsZDistribution->Fill(ext_hit.getPosition().z());

    }

    m_extHitsNum->Fill(counter);
    m_Hit2dsNum->Fill(m_hit2ds.getEntries());


    /********************************
      Hit2D z distrib
    **********************************/

    for (auto hit_2d : m_hit2ds) {
      m_Hit2dZDistrib->Fill(hit_2d.getPosition().z());

      // If Forward (2) else Backward(1)
      if (hit_2d.getSection() == 2) {
        m_Hit2dSectorHists[hit_2d.getSector()]->Fill(hit_2d.getLayer());
      } else {
        m_Hit2dSectorHists[hit_2d.getSector()]->Fill(-hit_2d.getLayer());
      }
    }

    //* END OF DEBUG ADDITIONAL CODE*//
  }
  //* End of Event *//
}

void EKLMTrackEffModule::terminate()
{

  m_MatchedDigitsInPlane->Sumw2();
  m_AllExtHitsInPlane->Sumw2();
  m_MatchedDigitsInPlane->Write();
  m_AllExtHitsInPlane->Write();

  m_planesEff->Divide(m_MatchedDigitsInPlane, m_AllExtHitsInPlane, 1, 1, "B");
  m_planesEff->Write();

  for (int i = 1; i < 5; i++) {
    m_MatchingByLayer[i]->Sumw2();
    // m_MatchingByLayer[i]->Write();
    m_AllExtHitsInLayer[i]->Sumw2();
    // m_AllExtHitsInLayer[i]->Write();
    m_LayersEff[i]->Divide(m_MatchingByLayer[i], m_AllExtHitsInLayer[i], 1, 1, "B");
    m_LayersEff[i]->Write();

    m_MatchingByPlaneVis[i]->Sumw2();
    // m_MatchingByPlaneVis[i]->Write();
    m_AllExtHitsInPlaneVis[i]->Sumw2();
    // m_AllExtHitsInPlaneVis[i]->Write();
    m_PlanesEffVis[i]->Divide(m_MatchingByPlaneVis[i], m_AllExtHitsInPlaneVis[i], 1, 1, "B");
    m_PlanesEffVis[i]->Write();
  }

  // Making 2d eff from 1d by sectors

  std::map<int, std::vector<double> > sectorEffErr2dFrom1d;
  std::map<int, std::vector<double> > sectorEff2dFrom1d;
  std::vector<double> layerNum;

  for (int i = -12; i < 14; i ++) {
    if (i < 0) layerNum.push_back(i);
    else layerNum.push_back(i + 1);
  }

  for (auto [sec, planeEffHist] : m_PlanesEffVis) {
    for (size_t i = 1; i < 54; i += 2) {
      if (planeEffHist->GetXaxis()->GetBinCenter(i) == 0.5) i++;   // To skip 0 empty bin
      sectorEff2dFrom1d[sec].push_back(planeEffHist->GetBinContent(i) * planeEffHist->GetBinContent(i + 1));
      sectorEffErr2dFrom1d[sec].push_back(std::sqrt(planeEffHist->GetBinError(i) * planeEffHist->GetBinError(i)
                                                    + planeEffHist->GetBinError(i + 1) * planeEffHist->GetBinError(i + 1)));
    }
  }

  std::map<int, TGraphErrors*> tGraphMapLayer;
  for (auto [sec, effVec] : sectorEff2dFrom1d) {
    tGraphMapLayer[sec] = new TGraphErrors(26, layerNum.data(), effVec.data(), nullptr,
                                           sectorEffErr2dFrom1d[sec].data());
    std::string graphName = "Layer efficiency sector " + std::to_string(sec) + " from 1d";
    tGraphMapLayer[sec]->SetName(graphName.data());
    tGraphMapLayer[sec]->Draw("AP*");
    tGraphMapLayer[sec]->GetYaxis()->SetRangeUser(0, 1.2);
    tGraphMapLayer[sec]->GetYaxis()->SetTitle("efficiency");
    tGraphMapLayer[sec]->GetXaxis()->SetTitle("Layer");
    tGraphMapLayer[sec]->Write();
  }

  if (m_debug) {
    // Writing all additional hists
    m_ExtHitsZDistribution->Write();
    m_ExtHitsLayerDistribution->Write();
    m_extHitsNum->Write();
    m_Hit2dsNum->Write();

    for (auto hist : m_ExtHitsSectorHists) {
      hist.second->Write();
    }

    m_ExtHitsZDistribTracks->Write();
    m_ExtHitsLayerDistribTracks->Write();

    for (auto hist : m_ExtHitsSectorHistsTrack) {
      hist.second->Write();
    }

    m_Hit2dZDistrib->Write();
    m_Hit2dMatchedDistrib->Write();

    for (auto hist : m_Hit2dSectorHists) {
      hist.second->Write();
    }

    m_MinHitDist->Write();
    m_DigitMinDist->Write();
    m_ExtHitZTheta->Write();
    m_ExtHitsCorrelation->Write();
    m_Hit2dsCorrelation->Write();
    m_ThetaCorrelationHist->Write();
    m_MuonsEnergy->Write();
    m_MuonsTheta->Write();
    m_MuonsThetaWithoutCut->Write();
    m_MuonsAngle->Write();
    m_D0Distribution->Write();
    m_Z0Distribution->Write();
  }

  m_file->Close();

}

