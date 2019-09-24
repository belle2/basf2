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

/* Belle2 headers. */
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <calibration/CalibrationCollectorModule.h>

namespace Belle2 {

  /**
   * Module EKLMTrackMatchCollectorModule.
   * @details
   * Module for collecting data for track matching efficiency.
   */

  class EKLMTrackMatchCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    EKLMTrackMatchCollectorModule();

    /**
     * Destructor.
     */
    virtual ~EKLMTrackMatchCollectorModule();

    /**
     * Initializer.
     */
    void prepare() override;

    /**
     * This method is called for each event.
     */
    void collect() override;

    /**
     * This method is called at the end of run.
     */
    void closeRun() override;

    /**
     * Was ExtHit entered in EKLM sensetive volume? If it isn`t returns tuple of -1.
     * If it is returns copyid, idSection, idLayer, idSector, idPlane, idStrip
     */
    std::tuple<int, int, int, int, int, int> checkExtHit(const ExtHit& ext_hit) const;

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
     * Find sum energy of tracks in event
     */
    double getSumTrackEnergy(const StoreArray<Track>& selected_tracks) const;

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

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** EKLMElementNumbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /** D0 and Z0 distance parameters */
    double m_D0Z0;

    /** Output file name */
    std::string m_filename;

    /** Max distance in strips number to 1D hit from extHit to be still matched */
    double m_AllowedDistance1D;

  };

}
