/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMAlignmentHit.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/TransformData.h>
#include <eklm/geometry/TransformDataGlobalAligned.h>


// ROOT includes
#include "TH1.h"
#include "TH2F.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"

// std lib includes

#include<map>

namespace Belle2 {

  /**
   * Module EKLMTrackEffModule.
   * @details
   * Module for monitoring track matching efficiency.
   */

  class EKLMTrackEffModule : public Module {

  public:

    EKLMTrackEffModule();

    virtual ~EKLMTrackEffModule();

    void initialize() override;

    void event() override;

    void terminate() override;

    // Was ExtHit entered in EKLM sensetive volume? If it isn`t returns tuple of -1.
    // If it is returns copyid, idEndcap, idLayer, idSector, idPlane, idStrip
    std::tuple<int, int, int, int, int, int> check_exthit(const ExtHit& ext_hit);

    // Find min distance from ext_hit to Hit2d in event
    double get_min_dist(const ExtHit& ext_hit);


    /*                     SIMPLE MUID
    Calculating number of Hit2ds in forward and backward parts
    If it is many hits in one of the endcaps can be sure that this muon track
    And the second track with high probability is in opposite endcap (because we choosed events with 2trks)
    If it is so, calculate efficiency in opposite endcap
    */
    std::pair<bool, bool> MuID(int number_of_required_hits);

    // Calculate hist errors
    double error_calc(int64_t num_of_hits, int64_t num_of_exthits);

    // Matching of digits with ext hits
    bool digits_matching(const ExtHit& ext_hit, double allowed_distance);

    // MAtching of Hit2ds wih exthits
    bool hit2ds_matching(const ExtHit& ext_hit, double allowed_distance);

    // Min dist (in number of strips) to ext hit from 1d hit
    double get_min_1d_dist(const ExtHit& ext_hit);

    //FInd sum energy of tracks
    double get_sum_track_energy(const StoreArray<Track>& selected_tracks);

    // Find angle between 2 tracks
    double angle_between_tracks(const StoreArray<Track>& selected_tracks);

    // Fiiling hist of correlation between tracks in event
    // Works only in case of 2 tracks!
    void ext_hits_corr(StoreArray<Track>& selected_tracks);
    void hit2ds_corr();
    void theta_corr(StoreArray<Track>& selected_tracks);


    // Function to determine sector efficiency from maps
    std::pair<std::map<int, std::vector<double> >, std::map<int, std::vector<double> > > calculate_sector_eff(
      std::map<int, std::map<int, int64_t> > sector_matching, std::map<int, std::map<int, int64_t> > sector_all);

    // Calculate distance to IP and make cut on this distance
    bool d0_z0_cut(const StoreArray<Track>& selected_tracks, double dist);

    // Making theta cut
    bool theta_acceptance(const StoreArray<Track>& selected_tracks);

  private:

    // Arrays for storing data objects
    StoreArray<EKLMDigit> digits;
    StoreArray<EKLMHit2d> hit2ds;
    StoreArray<Track> m_tracks;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<TrackFitResult> m_trackFitResults;
    StoreArray<ExtHit> m_extHits;
    StoreArray<EKLMAlignmentHit> hitAlign;
    StoreArray<KLMDigitEventInfo> m_DigitEventInfos;

    // Geometry
    const EKLM::ElementNumbersSingleton* m_eklmElementNumbers;
    const EKLM::GeometryData* m_GeoDat;
    const EKLM::TransformDataGlobalAligned* m_eklmTransformData;

    // Hist for z distribution of Hit2ds
    TH1F* Hit2d_z_distrib;
    TH1F* Hit2d_matched_distrib;
    std::map<int, TH1F*> Hit2d_sector_hists;

    // Hists for z distribution of ExtHits by sectors
    TH1F* Ext_hits_z_distrib;
    TH1F* Ext_hits_layer_distrib;
    std::map<int, TH1F*> sector_hists;

    // Hist for number of exthits per event
    TH1F* n_exthits;
    TH1F* n_Hit2ds;

    // Hists for z distribution of ExtHits with track and cuts
    TH1F* Ext_hits_z_distrib_tracks;
    TH1F* Ext_hits_layer_distrib_tracks;
    std::map<int, TH1F*> sector_hists_track;

    // Hist for tracks momentum
    TH1F* h_TrackMomentum;

    // Hist for z/theta distribution of extHits
    TH2F* ext_hit_z_theta;

    // Hits of minimal Hit2ds distance
    TH1F* min_hit_dist;

    // 1d hits min dist to exthit
    TH1F* hit1d_min_dist;

    // Corelation hist
    TH2F* hits_corr;
    TH2F* hit2ds_corr_hist;
    TH2F* theta_corr_hist;

    // Energy sum of tracks hist
    TH1F* muons_energy;

    // D0 and Z0 distributions
    TH1F* d0_distr;
    TH1F* z0_distr;

    // Tracks theta
    TH1F* muons_theta;
    TH1F* muons_theta_without_cut;
    TH1F* muons_angle;

    int m_eventCounter = 0;
    TFile* m_file;
    bool m_debug;
    double m_d0_z0;
    bool m_energy_cut;
    std::string m_filename;
    double m_minMass = 0.1;

    //Files for hists
    std::ofstream main_eff, layer_distrib, layer_track, hit2d_distrib, min_dist;


    // Maps to store layers info about matching of exthit and Hit2D; matching of exthit and Digit in one layer
    // If layer wit extHit have Hit2d - match

    std::map<int, int64_t> matching_by_layer;
    std::map<int, int64_t> all_exthits_in_layer;

    std::map<int, int64_t> matching_digits_by_plane;
    std::map<int, int64_t> all_exthits_in_plane;

    // Matchng for every sector and layer

    std::map<int, std::map<int, int64_t> > sector_matching_by_layer;
    std::map<int, std::map<int, int64_t> > sector_all_exthits_in_layer;

    std::map<int, std::map<int, int64_t> > sector_digit_matching_by_plane;
    std::map<int, std::map<int, int64_t> > all_digits_in_plane;

  };

}
