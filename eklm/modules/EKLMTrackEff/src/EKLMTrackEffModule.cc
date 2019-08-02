/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <CLHEP/Vector/LorentzVector.h>

#include <eklm/modules/EKLMTrackEff/EKLMTrackEffModule.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMAlignmentHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include "CLHEP/Geometry/Point3D.h"

// Std lib includes
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <tuple>

using namespace Belle2;

REG_MODULE(EKLMTrackEff)

EKLMTrackEffModule::EKLMTrackEffModule() : Module() , m_eventCounter(0)
{
  setDescription("Get track matching efficiency for EKLM");
  addParam("filename", m_filename, "Output root filename", std::string("EKLMTrackMatch.root"));
  addParam("debug", m_debug, "Debug/analysis mode", bool(false));
  addParam("z0_d0", m_d0_z0, "D0_z0 distance", double(5));
  // addParam("energy_cut", m_energy_cut, " 2 < E < 10 full energy", bool(false));
  // addParam("minDiMuonMass", m_minMass, "Minimal dimuon invariant mass", 0.1);
}

EKLMTrackEffModule::~EKLMTrackEffModule()
{

}

/* Define some functions in EKLMTrackEffModule namespace
  and then use them in EKLMTrackEffModule::event like in
  main() function. That helps support code readable and free to modify */

void EKLMTrackEffModule::initialize()
{
  digits.isRequired();
  hit2ds.isRequired();
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_extHits.isRequired();
  hitAlign.isRequired();
  m_tracks.registerRelationTo(hit2ds);
  m_recoTracks.registerRelationTo(hit2ds);
  digits.registerRelationTo(m_DigitEventInfos);
  m_GeoDat = &(EKLM::GeometryData::Instance());
  // hit2ds.registerRelation(<Tracks>);

  // Set of declarated variables

  m_file = new TFile(m_filename.c_str(), "recreate");
  m_file->cd();


  //* For Debug / analysis *//

  if (m_debug) {
    //Files for hists
    layer_distrib.open("layer_distrib.txt");
    layer_track.open("layer_track.txt");
    hit2d_distrib.open("hit2d_distrib.txt");
    min_dist.open("min_dist.txt");
  }

  Ext_hits_z_distrib = new TH1F("z_distrib_without_tracks", "", 600, -300, 420);
  n_exthits = new TH1F("Number_of_exthits", "", 59, 1, 60);
  n_Hit2ds = new TH1F("Number_of_Hit2ds", "", 59, 1, 60);

  ext_hit_z_theta = new TH2F("z/theta distribution of extHits", "", 26, -12, 14, 90, 0, 180);
  ext_hit_z_theta->GetXaxis()->SetTitle("Layer");
  ext_hit_z_theta->GetYaxis()->SetTitle("Theta");

  for (int i = 1; i < 5; ++i) {
    std::string name = "Sector " + std::to_string(i);
    sector_hists[i] = new TH1F(name.data(), " ", 40, -20, 20);
  }

  Ext_hits_z_distrib_tracks = new TH1F("z_distrib_with_tracks", "", 600, -300, 420);

  for (int i = 1; i < 5; ++i) {
    std::string name = "With track by Sector " + std::to_string(i);
    sector_hists_track[i] = new TH1F(name.data(), " ", 40, -20, 20);
  }


  Hit2d_z_distrib = new TH1F("Hit2d_z_distib", "", 600, -300, 420);

  for (int i = 1; i < 5; ++i) {
    std::string name = "Hit2D with Sector " + std::to_string(i);
    Hit2d_sector_hists[i] = new TH1F(name.data(), " ", 40, -20, 20);
  }

  h_TrackMomentum = new TH1F("momentum", "", 200, 0, 10);

  min_hit_dist = new TH1F("min_dist", "", 50, 0, 50);
  hit1d_min_dist = new TH1F("min dist for 1d", "", 20, 0, 20);

  Ext_hits_layer_distrib = new TH1F("ext_hit_layer_distrib", "", 26, -12, 14);

  Hit2d_matched_distrib = new TH1F("Hit2ds_matched_distrib", "", 26, -12, 14);

  Ext_hits_layer_distrib_tracks = new TH1F("Ext_hits_layer_distrib_track", "", 26, -12, 14);

  hits_corr = new TH2F("Corr_between_hits_in_tracks", "", 34, 1, 35, 34, 1, 35);

  hit2ds_corr_hist = new TH2F("Corr_between_hit2ds_int_back_and_fwd", "", 14, 1, 15, 14, 1, 15);
  hit2ds_corr_hist->GetYaxis()->SetTitle("Bwd");
  hit2ds_corr_hist->GetXaxis()->SetTitle("Fwd");

  theta_corr_hist = new TH2F("Theta_corr", "", 45, 0, 180, 45, 0, 180);
  muons_energy = new TH1F("Muons energy", "", 100, 1, 12);
  muons_theta = new TH1F("Muons theta", "", 45, 0, 180);
  muons_theta_without_cut = new TH1F("Muons theta witout cut", "", 45, 0, 180);
  muons_angle = new TH1F("Angle between muons", "", 30, 0, CLHEP::pi);

  d0_distr = new TH1F("D0", "", 120, 0, 6);
  z0_distr = new TH1F("Z0", "", 120, 0, 6);
}

std::tuple<int, int, int, int, int, int> EKLMTrackEffModule::check_exthit(const ExtHit& ext_hit)
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

double EKLMTrackEffModule::get_min_dist(const ExtHit& ext_hit)
{
  double distance_value = 0;
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);

  for (auto hit_2_d : hit2ds) {
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

double EKLMTrackEffModule::get_min_1d_dist(const ExtHit& ext_hit)
{
  double distance_value = -1;
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);

  for (auto hit_1d : digits) {
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

std::pair<bool, bool> EKLMTrackEffModule::MuID(int number_of_required_hits)
{
  // Map to calc number of hits
  // Forward - 2, backward - 1
  bool mark_forward = false;
  bool mark_backward = false;

  std::map<int, int> endcap_hits = { {1, 0}, {2, 0} };
  for (auto hit_ : hit2ds) {
    endcap_hits[hit_.getEndcap()]++;
  }

  if (endcap_hits[1] > number_of_required_hits) mark_forward = true;
  if (endcap_hits[2] > number_of_required_hits) mark_backward = true;

  return std::make_pair(mark_forward, mark_backward);
}

double EKLMTrackEffModule::error_calc(int64_t num_of_hits, int64_t num_of_exthits)
{
  double hit2d_err = std::sqrt(static_cast<long double>(num_of_hits)) / num_of_hits;
  double exthits_err = std::sqrt(static_cast<long double>(num_of_exthits)) / num_of_exthits;
  double error = (std::sqrt(hit2d_err * hit2d_err + exthits_err * exthits_err));
  return error;
}

bool EKLMTrackEffModule::digits_matching(const ExtHit& ext_hit, double allowed_distance)
{
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);
  TVector3 ext_hit_pos = ext_hit.getPosition();

  for (auto hit_1d : digits) {
    TVector3 hit1d_pos = hit_1d.getPosition();
    TVector3 distance = hit1d_pos - ext_hit_pos;

    if (hit_1d.getLayer() == idLayer && hit_1d.getEndcap() == idEndcap
        && hit_1d.getSector() == idSector && hit_1d.getPlane() == idPlane
        && (idStrip > hit_1d.getStrip() - allowed_distance && idStrip < hit_1d.getStrip() + allowed_distance))
    { return true; }
  }

  return false;
}

bool EKLMTrackEffModule::hit2ds_matching(const ExtHit& ext_hit, double allowed_distance)
{
  int copyid = ext_hit.getCopyID();
  int idEndcap = -1, idLayer = -1, idSector = -1, idPlane = -1, idStrip = -1;
  m_GeoDat->stripNumberToElementNumbers(copyid, &idEndcap, &idLayer, &idSector, &idPlane, &idStrip);
  TVector3 ext_hit_pos = ext_hit.getPosition();

  for (auto hit_2d : hit2ds) {
    TVector3 hit1d_pos = hit_2d.getPosition();
    TVector3 distance = hit1d_pos - ext_hit_pos;

    if (hit_2d.getLayer() == idLayer && hit_2d.getEndcap() == idEndcap
        && hit_2d.getSector() == idSector && distance.Mag() < allowed_distance) { return true; }
  }

  return false;
}

void EKLMTrackEffModule::ext_hits_corr(StoreArray<Track>& selected_tracks)
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

  hits_corr->Fill(first_trk, second_trk);
}

void EKLMTrackEffModule::hit2ds_corr()
{
  int bwd_hits = 0;
  int fwd_hits = 0;
  for (auto hit : hit2ds) {
    if (hit.getEndcap() == 1) bwd_hits++;
    if (hit.getEndcap() == 2) fwd_hits++;
  }
  hit2ds_corr_hist->Fill(fwd_hits, bwd_hits);
}

void EKLMTrackEffModule::theta_corr(StoreArray<Track>& selected_tracks)
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
  theta_corr_hist->Fill(first_trk, second_trk);
}

double EKLMTrackEffModule::get_sum_track_energy(const StoreArray<Track>& selected_tracks)
{
  double eneregy = 0;
  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    eneregy += fitResult->getEnergy();
  }
  return eneregy;
}

double EKLMTrackEffModule::angle_between_tracks(const StoreArray<Track>& selected_tracks)
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

std::pair<std::map<int, std::vector<double> >, std::map<int, std::vector<double> > >
EKLMTrackEffModule::calculate_sector_eff(std::map<int, std::map<int, int64_t> > sector_matching,
                                         std::map<int, std::map<int, int64_t> > sector_all)
{

  std::map<int, std::vector<double> > sector_layers_eff;
  std::map<int, std::vector<double> > sector_layers_eff_err;
  for (auto [sector, sectors_eff] : sector_matching) {
    std::vector<double> layer_eff_temp;
    std::vector<double> layer_eff_temp_err;
    for (auto [layer, num_of_hits] : sectors_eff) {
      int64_t num_of_exthits = sector_all[sector][layer];
      double partial_eff = static_cast<long double>(num_of_hits) / num_of_exthits;
      layer_eff_temp.push_back(partial_eff);
      layer_eff_temp_err.push_back(error_calc(num_of_hits, num_of_exthits));
    }
    sector_layers_eff[sector] = layer_eff_temp;
    sector_layers_eff_err[sector] = layer_eff_temp_err;
  }

  return std::make_pair(sector_layers_eff, sector_layers_eff_err);
}

bool EKLMTrackEffModule::d0_z0_cut(const StoreArray<Track>& selected_tracks, double dist)
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

bool EKLMTrackEffModule::theta_acceptance(const StoreArray<Track>& selected_tracks)
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
  auto [mark_forward, mark_backward] = MuID(5);

  for (const Track& track : m_tracks) {

    // Making different cuts
    const RecoTrack* recoTrack = track.getRelated<RecoTrack>();

    if (recoTrack && m_tracks.getEntries() == 2) {
      const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::muon);
      if (fitResult->getMomentum().Mag() > 11. || fitResult->getMomentum().Mag() < 1.) continue;
      // if (m_energy_cut) {
      //   if (get_sum_track_energy(m_tracks) > 10) continue;
      // }

      if (d0_z0_cut(m_tracks, m_d0_z0)) continue;
      // if (theta_acceptance(m_tracks)) continue;

      for (const auto& ext_hit : track.getRelationsTo<ExtHit>()) {

        auto [copyid, idEndcap, idLayer, idSector, idPlane, idStrip] = check_exthit(ext_hit);
        if (copyid == -1) continue;

        TVector3 ext_hit_pos = ext_hit.getPosition();

        // Max distance to 2D hit
        double allowed_distance2D = 15;
        // Max distance to 1D hit (in strips)
        double allowed_distance1D = 8;

        if (idEndcap == 2 && mark_forward) {
          if (hit2ds_matching(ext_hit, allowed_distance2D)) {
            matching_by_layer[idLayer]++;
            sector_matching_by_layer[idSector][idLayer]++;
            Hit2d_matched_distrib->Fill(idLayer);
          }

          all_exthits_in_layer[idLayer]++;
          sector_all_exthits_in_layer[idSector][idLayer]++;

          if (digits_matching(ext_hit, allowed_distance1D)) {
            matching_digits_by_plane[ 2 * idLayer + idPlane - 2]++;
            sector_digit_matching_by_plane[idSector][ 2 * idLayer + idPlane - 2]++;
          }

          all_exthits_in_plane[2 * idLayer + idPlane - 2]++;
          all_digits_in_plane[idSector][ 2 * idLayer + idPlane - 2]++;

        } else if (idEndcap == 1 && mark_backward) {
          if (hit2ds_matching(ext_hit, allowed_distance2D)) {
            matching_by_layer[-idLayer]++;
            sector_matching_by_layer[idSector][-idLayer]++;
            Hit2d_matched_distrib->Fill(-idLayer);
          }

          all_exthits_in_layer[-idLayer]++;
          sector_all_exthits_in_layer[idSector][-idLayer]++;

          if (digits_matching(ext_hit, allowed_distance1D)) {
            matching_digits_by_plane[-2 * idLayer - idPlane + 2]++;
            sector_digit_matching_by_plane[idSector][ - 2 * idLayer - idPlane + 2]++;
          }

          all_exthits_in_plane[-2 * idLayer - idPlane + 2]++;
          all_digits_in_plane[idSector][ - 2 * idLayer - idPlane + 2]++;

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
        muons_theta_without_cut->Fill(fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi);
        // if (m_energy_cut) {
        // if (get_sum_track_energy(m_tracks) > 10) continue;
        // }
        double d0 = fitResult->getD0();
        double z0 = fitResult->getZ0();
        d0_distr->Fill(d0);
        z0_distr->Fill(z0);
        if (d0_z0_cut(m_tracks, m_d0_z0)) continue;
        // if (theta_acceptance(m_tracks)) continue;
        muons_theta->Fill(fitResult->getMomentum().Theta() * 180.0 / CLHEP::pi);

        // Only in debug mode
        // Searching for correlations
        // Use this functions only when second track is OK
        if (trk_cnt == 2) {
          ext_hits_corr(m_tracks);
          hit2ds_corr();
          theta_corr(m_tracks);
          //Get tracks energy distib
          muons_energy->Fill(get_sum_track_energy(m_tracks));
          muons_angle->Fill(angle_between_tracks(m_tracks));
        }

        for (const auto& ext_hit : track.getRelationsTo<ExtHit>()) {

          auto [copyid, idEndcap, idLayer, idSector, idPlane, idStrip] = check_exthit(ext_hit);
          if (copyid == -1) continue;

          TVector3 ext_hit_pos = ext_hit.getPosition();
          double theta = ext_hit_pos.Theta() * 180.0 / CLHEP::pi;
          // double theta = (ext_hit_pos.Phi() < CLHEP::pi / 2 && ext_hit_pos.Phi() > -CLHEP::pi / 2 ) ? ext_hit_pos.Theta() * 180.0 / CLHEP::pi : -ext_hit_pos.Theta() * 180.0 / CLHEP::pi;

          // Maybe only EKLM zone?
          // if (theta < 25 || theta > 37 ) {
          //   if (theta < 130 || theta > 145) break;
          // }

          // If Forward (2) else Backward(1)
          if (idEndcap == 2 && mark_forward) {
            sector_hists_track[idSector]->Fill(idLayer);
            Ext_hits_layer_distrib_tracks->Fill(idLayer);
            layer_track << idLayer << '\n';
            Ext_hits_z_distrib_tracks->Fill(ext_hit.getPosition().z());
            // To get all exthits in event
            ext_hit_z_theta->Fill(idLayer, theta);
          } else if (idEndcap == 1 && mark_backward) {
            sector_hists_track[idSector]->Fill(-idLayer);
            Ext_hits_layer_distrib_tracks->Fill(-idLayer);
            layer_track << -idLayer << '\n';
            Ext_hits_z_distrib_tracks->Fill(ext_hit.getPosition().z());
            ext_hit_z_theta->Fill(-idLayer, theta);
          }

          // To get distance distribution
          min_hit_dist->Fill(get_min_dist(ext_hit));
          hit1d_min_dist->Fill(get_min_1d_dist(ext_hit));
          min_dist << get_min_dist(ext_hit) << '\n';

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

      auto [copyid, idEndcap, idLayer, idSector, idPlane, idStrip] = check_exthit(ext_hit);
      if (copyid == -1) continue;

      // If Forward (2) else Backward(1)
      if (idEndcap == 2) {
        Ext_hits_layer_distrib->Fill(idLayer);
        layer_distrib << idLayer << '\n';
        sector_hists[idSector]->Fill(idLayer);
      } else {
        Ext_hits_layer_distrib->Fill(-idLayer);
        layer_distrib << -idLayer << '\n';
        sector_hists[idSector]->Fill(-idLayer);
      }

      ++counter;
      Ext_hits_z_distrib->Fill(ext_hit.getPosition().z());

    }

    n_exthits->Fill(counter);
    n_Hit2ds->Fill(hit2ds.getEntries());


    /********************************
      Hit2D z distrib
    **********************************/

    for (auto hit_2d : hit2ds) {
      Hit2d_z_distrib->Fill(hit_2d.getPosition().z());

      // If Forward (2) else Backward(1)
      if (hit_2d.getEndcap() == 2) {
        hit2d_distrib << hit_2d.getLayer() << '\n';
        Hit2d_sector_hists[hit_2d.getSector()]->Fill(hit_2d.getLayer());
      } else {
        Hit2d_sector_hists[hit_2d.getSector()]->Fill(-hit_2d.getLayer());
        hit2d_distrib << -hit_2d.getLayer() << '\n';
      }
    }

    //* END OF DEBUG ADDITIONAL CODE*//
  }
  //* End of Event *//
}

void EKLMTrackEffModule::terminate()
{

  // Layer eff
  std::vector<double> layer_eff;
  std::vector<double> layer_num;
  std::vector<double> error;
  std::vector<double> error_x;
  error_x.resize(26);
  for (auto [layer, num_of_hits] : matching_by_layer) {
    int64_t num_of_exthits = all_exthits_in_layer[layer];

    // Calculating errors
    error.push_back(error_calc(num_of_hits, num_of_exthits));
    double partial_eff = static_cast<long double>(num_of_hits) / num_of_exthits;
    layer_eff.push_back(partial_eff);
    layer_num.push_back(layer);
  }

  // Calculating layer eff by sectors
  auto [sector_layers_eff, sector_layers_eff_err] = calculate_sector_eff(sector_matching_by_layer, sector_all_exthits_in_layer);

  TGraphErrors* Ext_2Dhits_layer_efficiency = new TGraphErrors(static_cast<Int_t>(layer_num.size()), layer_num.data(),
      layer_eff.data(), error_x.data(), error.data());
  Ext_2Dhits_layer_efficiency->SetName("layer_eff");
  Ext_2Dhits_layer_efficiency->Draw("AP*");
  Ext_2Dhits_layer_efficiency->GetYaxis()->SetRangeUser(0, 1.2);
  Ext_2Dhits_layer_efficiency->GetYaxis()->SetTitle("efficiency");
  Ext_2Dhits_layer_efficiency->GetXaxis()->SetTitle("Layer");
  Ext_2Dhits_layer_efficiency->Write();

  std::map<int, TGraphErrors*> tgraph_map;
  for (auto [sec, eff_vec] : sector_layers_eff) {
    tgraph_map[sec] = new TGraphErrors(static_cast<Int_t>(layer_num.size()), layer_num.data(), eff_vec.data(), nullptr,
                                       sector_layers_eff_err[sec].data());
    std::string graph_name = "layer_eff_sector_" + std::to_string(sec);
    tgraph_map[sec]->SetName(graph_name.data());
    tgraph_map[sec]->Draw("AP*");
    tgraph_map[sec]->GetYaxis()->SetRangeUser(0, 1.2);
    tgraph_map[sec]->GetYaxis()->SetTitle("efficiency");
    tgraph_map[sec]->GetXaxis()->SetTitle("Layer");
    tgraph_map[sec]->Write();
  }

  error = {};
  layer_eff = {};
  layer_num = {};
  error_x.resize(52);

  for (auto [plane, num_of_hits] : matching_digits_by_plane) {
    int64_t num_of_exthits = all_exthits_in_plane[plane];

    // Calculating errors
    error.push_back(error_calc(num_of_hits, num_of_exthits));
    double partial_eff = static_cast<long double>(num_of_hits) / num_of_exthits;
    layer_eff.push_back(partial_eff);
    layer_num.push_back(plane);
  }

  TGraphErrors* plane_eff = new TGraphErrors(static_cast<Int_t>(layer_num.size()), layer_num.data(),
                                             layer_eff.data(), error_x.data(), error.data());
  plane_eff->SetName("plane_eff");
  plane_eff->Draw("AP*");
  plane_eff->GetYaxis()->SetRangeUser(0, 1.2);
  plane_eff->GetYaxis()->SetTitle("efficiency");
  plane_eff->GetXaxis()->SetTitle("2 * Layer + plane - 2");
  plane_eff->Write();

  // Plane eff by sectors

  auto [sector_plane_eff, sector_plane_eff_err] = calculate_sector_eff(sector_digit_matching_by_plane, all_digits_in_plane);

  std::map<int, TGraphErrors*> tgraph_map_plane;
  for (auto [sec, eff_vec] : sector_plane_eff) {
    tgraph_map_plane[sec] = new TGraphErrors(static_cast<Int_t>(layer_num.size()), layer_num.data(), eff_vec.data(), nullptr,
                                             sector_plane_eff_err[sec].data());
    std::string graph_name = "layer_eff_sector_" + std::to_string(sec) + " from 1d";
    tgraph_map_plane[sec]->SetName(graph_name.data());
    tgraph_map_plane[sec]->Draw("AP*");
    tgraph_map_plane[sec]->GetYaxis()->SetRangeUser(0, 1.2);
    tgraph_map_plane[sec]->GetYaxis()->SetTitle("efficiency");
    tgraph_map_plane[sec]->GetXaxis()->SetTitle("2 * Layer + plane - 2");
    tgraph_map_plane[sec]->Write();
  }


  // Making 2d eff from 1d:
  error_x.resize(26);
  std::vector<double> error_y;
  layer_num = {};
  for (int i = -12; i < 14; i++) {
    if (i > -1) layer_num.push_back(i + 1);
    else layer_num.push_back(i);
  }

  std::vector<double> eff_2d_from1d;
  for (size_t i = 0; i < layer_eff.size(); i += 2) {
    eff_2d_from1d.push_back(layer_eff.at(i) * layer_eff.at(i + 1));
    error_y.push_back(std::sqrt(error.at(i) * error.at(i) + error.at(i + 1) * error.at(i + 1)));
  }

  TGraphErrors* from_1d_to_2d = new TGraphErrors(static_cast<Int_t>(layer_num.size()), layer_num.data(),
                                                 eff_2d_from1d.data(), error_x.data(), error_y.data());
  from_1d_to_2d->SetName("2d_from_1d_eff");
  from_1d_to_2d->Draw("AP*");
  from_1d_to_2d->GetYaxis()->SetRangeUser(0, 1.2);
  from_1d_to_2d->GetYaxis()->SetTitle("efficiency");
  from_1d_to_2d->GetXaxis()->SetTitle("Layer");
  from_1d_to_2d->Write();

  // Making 2d eff from 1d by sectors

  error_x.resize(26);
  std::map<int, std::vector<double> > sector_eff_err_2d_from_1d;
  std::map<int, std::vector<double> > sector_eff_2d_from_1d;

  for (auto [sec, plane_eff_vec] : sector_plane_eff) {
    for (size_t i = 0; i < plane_eff_vec.size(); i += 2) {
      sector_eff_2d_from_1d[sec].push_back(plane_eff_vec.at(i) * plane_eff_vec.at(i + 1));
      sector_eff_err_2d_from_1d[sec].push_back(std::sqrt(sector_plane_eff_err[sec].at(i) * sector_plane_eff_err[sec].at(i)
                                                         + sector_plane_eff_err[sec].at(i + 1) * sector_plane_eff_err[sec].at(i + 1)));
    }
  }

  for (auto [sec, eff_vec] : sector_eff_2d_from_1d) {
    tgraph_map_plane[sec] = new TGraphErrors(static_cast<Int_t>(layer_num.size()), layer_num.data(), eff_vec.data(), nullptr,
                                             sector_eff_err_2d_from_1d[sec].data());
    std::string graph_name = "plane_eff_sector_" + std::to_string(sec) + " from_1d";
    tgraph_map_plane[sec]->SetName(graph_name.data());
    tgraph_map_plane[sec]->Draw("AP*");
    tgraph_map_plane[sec]->GetYaxis()->SetRangeUser(0, 1.2);
    tgraph_map_plane[sec]->GetYaxis()->SetTitle("efficiency");
    tgraph_map_plane[sec]->GetXaxis()->SetTitle("Layer");
    tgraph_map_plane[sec]->Write();
  }



  if (m_debug) {
    // Writing all additional hists
    Ext_hits_z_distrib->Write();
    Ext_hits_layer_distrib->Write();
    n_exthits->Write();
    n_Hit2ds->Write();

    for (auto hist : sector_hists) {
      hist.second->Write();
    }

    Ext_hits_z_distrib_tracks->Write();
    Ext_hits_layer_distrib_tracks->Write();

    for (auto hist : sector_hists_track) {
      hist.second->Write();
    }

    Hit2d_z_distrib->Write();
    Hit2d_matched_distrib->Write();

    for (auto hist : Hit2d_sector_hists) {
      hist.second->Write();
    }

    min_hit_dist->Write();
    hit1d_min_dist->Write();
    ext_hit_z_theta->Write();
    h_TrackMomentum->Write();
    hits_corr->Write();
    hit2ds_corr_hist->Write();
    theta_corr_hist->Write();
    muons_energy->Write();
    muons_theta->Write();
    muons_theta_without_cut->Write();
    muons_angle->Write();
    d0_distr->Write();
    z0_distr->Write();
  }

  m_file->Close();

}

