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

/* KLM headers. */
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <calibration/CalibrationCollectorModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  /**
   * Module EKLMTrackMatchCollectorModule.
   * @details
   * Module for collecting data for track matching efficiency.
   */

  class EKLMTrackMatchCollectorModule : public CalibrationCollectorModule {

  private:

    /**
     * Hit data.
     */
    struct HitData {

      /** Section. */
      int section;

      /** Layer. */
      int layer;

      /** Sector. */
      int sector;

      /** Plane. */
      int plane;

      /** Strip. */
      int strip;

      /** Extrapolation hit. */
      const ExtHit* hit;

      /** Digit. */
      const EKLMDigit* digit;

    };

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

    /**                     SIMPLE MUID
     * Calculating number of Hit2ds in forward and backward parts
     * If there are many hits in one of the sections can be sure that this is muon track
     * And the second track with high probability is in opposite section (because we choosed events with 2trks)
     * If it is so, calculate efficiency in opposite section
     */
    void trackCheck(
      bool trackSelected[EKLMElementNumbers::getMaximalSectionNumber()],
      int requiredHits) const;

    /**
     * Matching of digits with ext hits
     * @param[in] hitData         Hit data.
     * @param[in] allowedDistance Minimal distance in the units of strip number.
     */
    const EKLMDigit* findMatchingDigit(const struct HitData* hitData,
                                       double allowedDistance) const;


    /**
     * Find sum energy of tracks in event
     */
    double getSumTrackEnergy(const StoreArray<Track>& selected_tracks) const;

  private:

    /**
     * Collect the data for one track.
     * @param[in] track Track.
     */
    void collectDataTrack(const Track* track);

    /** Muon list name. If empty, use tracks. */
    std::string m_MuonListName;

    /**
     * Whether to use standalone track selection.
     * Always turn this off for cosmic data.
     */
    bool m_StandaloneTrackSelection;

    /**
     * Minimal number of matching digits.
     */
    int m_MinimalMatchingDigits;

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

    /** Muons. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** EKLMElementNumbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /** Output file name */
    std::string m_filename;

    /** Max distance in strips number to 1D hit from extHit to be still matched */
    double m_AllowedDistance1D;

  };

}
