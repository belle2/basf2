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

/* C++ headers. */
#include<map>

/* ROOT headers. */
#include "TH1.h"
#include "TH2F.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <klm/eklm/dataobjects/EKLMAlignmentHit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>
#include <klm/eklm/geometry/TransformDataGlobalAligned.h>

namespace Belle2 {

  /**
   * Module EKLMTrackEffModule.
   * @details
   * Module for monitoring track matching efficiency.
   */

  class EKLMTrackEffModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMTrackEffModule();

    /**
     * Destructor.
     */
    virtual ~EKLMTrackEffModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called in the end of event processing.
     */
    void terminate() override;

    /**
     * Was ExtHit entered in EKLM sensetive volume? If it isn`t returns tuple of -1.
     * If it is returns copyid, idSection, idLayer, idSector, idPlane, idStrip
     */
    std::tuple<int, int, int, int, int, int> checkExtHit(const ExtHit& ext_hit) const;

    /**
     * Find min distance from ext_hit to Hit2d in event
     */
    double getMinDist(const ExtHit& ext_hit) const;

    /**
     * Find dist (in number of strips) to ext hit from 1d hit
     */
    double getMinDist1d(const ExtHit& ext_hit) const;


    /**                     SIMPLE MUID
     * Calculating number of Hit2ds in forward and backward parts
     * If there are many hits in one of the sections can be sure that this is muon track
     * And the second track with high probability is in opposite section (because we choosed events with 2trks)
     * If it is so, calculate efficiency in opposite section
     */
    std::pair<bool, bool> trackCheck(int number_of_required_hits) const;

    /**
     *  Matching of digits with ext hits
     */
    bool digitsMatching(const ExtHit& ext_hit, double allowed_distance) const;

    /**
     * Matching of Hit2ds wih exthits
     */
    bool hit2dsMatching(const ExtHit& ext_hit, double allowed_distance) const;

    /**
     * Find sum energy of tracks in event
     */
    double getSumTrackEnergy(const StoreArray<Track>& selected_tracks) const;

    /**
     * Find theta angle between 2 tracks
     */
    double angleThetaBetweenTracks(const StoreArray<Track>& selected_tracks) const;

    /**
     * Fiiling hist of correlation between extHits of tracks in event
     * Works only in case of 2 tracks!
     */
    void extHitsCorr(const StoreArray<Track>& selected_tracks);

    /**
     * Fiiling hist of correlation between Hit2ds in event
     * Works only in case of 2 tracks!
     */
    void hit2dsCorr();

    /**
     * Fiiling hist of correlation between theta angle of tracks in event
     * Works only in case of 2 tracks!
     */
    void thetaCorr(const StoreArray<Track>& selected_tracks);

    /**
     * Calculate distance to IP and make cut on this distance
     */
    bool d0z0Cut(const StoreArray<Track>& selected_tracks, double dist) const;

    /**
     * Making theta cut
     */
    bool thetaAcceptance(const StoreArray<Track>& selected_tracks) const;

  private:

    /** Digits. */
    StoreArray<EKLMDigit> m_digits;

    /** Hit2ds. */
    StoreArray<EKLMHit2d> m_hit2ds;

    /** Tracks. */
    StoreArray<Track> m_tracks;

    /** RecoTracks. */
    StoreArray<RecoTrack> m_recoTracks;

    /** TrackFitResult. */
    StoreArray<TrackFitResult> m_trackFitResults;

    /** ExtHits. */
    StoreArray<ExtHit> m_extHits;

    /** AlignmentHits. */
    StoreArray<EKLMAlignmentHit> m_hitAlign;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** EKLM Element Numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /** Hist for z distribution of Hit2ds */
    TH1F* m_Hit2dZDistrib;

    /** Hist for layers distribution of matched Hit2ds */
    TH1F* m_Hit2dMatchedDistrib;

    /** Map of hists for layers distribution of matched Hit2ds by sectors*/
    std::map<int, TH1F*> m_Hit2dSectorHists;

    /** Hist for z distribution of ExtHits */
    TH1F* m_ExtHitsZDistribution;

    /** Hist for layer distribution of ExtHits */
    TH1F* m_ExtHitsLayerDistribution;

    /** Hists for z distribution of ExtHits by layer */
    std::map<int, TH1F*> m_ExtHitsSectorHists;

    /** Hist for number of exthits per event */
    TH1F* m_extHitsNum;

    /** Hist for number of Hit2ds per event */
    TH1F* m_Hit2dsNum;

    /** Hist for z distribution of ExtHits with track and cuts */
    TH1F* m_ExtHitsZDistribTracks;

    /** Hist for layer distribution of ExtHits with track and cuts */
    TH1F* m_ExtHitsLayerDistribTracks;

    /** Hists for layer distribution of ExtHits with track and cuts by sector */
    std::map<int, TH1F*> m_ExtHitsSectorHistsTrack;

    /** Hist for z/theta distribution of extHits */
    TH2F* m_ExtHitZTheta;

    /** Hits of minimal distance between Hit2d and extHit*/
    TH1F* m_MinHitDist;

    /** Hits of minimal distance between digit and extHit */
    TH1F* m_DigitMinDist;

    /** Correlation between ExtHits of 2 tracks in event */
    TH2F* m_ExtHitsCorrelation;

    /** Correlation between Hit2ds of 2 tracks in event */
    TH2F* m_Hit2dsCorrelation;

    /** Correlation between Hit2ds of 2 tracks in event */
    TH2F* m_ThetaCorrelationHist;

    /** Energy sum of tracks in event */
    TH1F* m_MuonsEnergy;

    /** D0 distribution*/
    TH1F* m_D0Distribution;

    /** Z0 distribution*/
    TH1F* m_Z0Distribution;

    /** Muons tracks theta */
    TH1F* m_MuonsTheta;

    /** Muons tracks theta without cut */
    TH1F* m_MuonsThetaWithoutCut;

    /** Theta angle between muons tracks */
    TH1F* m_MuonsAngle;

    /** File with the result */
    TFile* m_file;

    /** Run in debug mode or not */
    bool m_debug;

    /** D0 and Z0 distance parameters */
    double m_D0Z0;

    /** Output file name */
    std::string m_filename;

    /** Max distance to 2D hit from extHit to be still matched */
    double m_AllowedDistance2D;

    /** Max distance in strips number to 1D hit from extHit to be still matched */
    double m_AllowedDistance1D;

    /** Hist for containing matched digits for each plane (data for calibration) */
    TH1F* m_MatchedDigitsInPlane;

    /** Hist for containing all extHits for each plane (data for calibration) */
    TH1F* m_AllExtHitsInPlane;

    /** Hist of planes eff (data for calibration) */
    TH1F* m_planesEff;

    /** Hists to store info about matching of exthit and Hit2D in layer for each sector */
    std::map<int, TH1F*> m_MatchingByLayer;

    /** Hists to store info about all of exthit in layer for each sector */
    std::map<int, TH1F*> m_AllExtHitsInLayer;

    /** Hist of layers eff (data for visualisation) by sectors*/
    std::map<int, TH1F*> m_LayersEff;

    /** Hists to store info about matching of exthit and Digits in plane for each sector */
    std::map<int, TH1F*> m_MatchingByPlaneVis;

    /** Hists to store info about all of exthit in plane for each sector */
    std::map<int, TH1F*> m_AllExtHitsInPlaneVis;

    /** Hist of planes eff (data for visualisation) by sector */
    std::map<int, TH1F*> m_PlanesEffVis;

  };

}
