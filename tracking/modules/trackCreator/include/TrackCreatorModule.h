/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <tracking/dbobjects/TrackFitMomentumRange.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/geometry/B2Vector3.h>
#include <vector>
#include <string>

namespace Belle2 {

  /**
   * Takes RecoTracks coming from the event reconstructions and fits them
   * with the configured list of particles hypothesis and stores the
   * result in MDST-usable Belle2::Track and Belle2::TrackFitResult classes
   */
  class TrackCreatorModule : public Module {

  public:
    /// Constructor adding the description and properties.
    TrackCreatorModule();

    /// Require and register the store arrays.
    void initialize() override;

    /// Called when entering a new run, to load TrackFitMomentumRange parameters.
    void beginRun() override;

    /// Build/fit the track fit results.
    void event() override;

  private:
    // Input
    /// Name of collection holding the RecoTracks (input).
    std::string m_recoTrackColName = "";

    // Output
    /// Name of collection holding the Tracks (output).
    std::string m_trackColName = "";
    /// Name of collection holding the TrackFitResult (output).
    std::string m_trackFitResultColName = "";

    /// BeamSpot define the coordinate system in which the tracks will be extrapolated to the perigee.
    std::vector<double> m_beamSpot = {0.0, 0.0, 0.0};
    /// BeamSpot as B2Vector3D
    B2Vector3D m_beamSpotAsTVector;
    /// BeamAxis define the coordinate system in which the tracks will be extrapolated to the perigee.
    std::vector<double> m_beamAxis = {0.0, 0.0, 1.0};
    /// BeamAxis as B2Vector3D
    B2Vector3D m_beamAxisAsTVector;
    /// PDG codes for which TrackFitResults will be created.
    std::vector<int> m_pdgCodes = {211};

    /// Flag to turn on special handling which measurement to choose; especially useful for Cosmics.
    bool m_useClosestHitToIP = false;
    /**
      * Flag to calculate the BField at the used hit (closest to IP or first one), instead of the one at the POCA.
      * Use this for cosmics to prevent problems, when cosmics reconstruction end up in the QCS magnet.
      */
    bool m_useBFieldAtHit = false;

    /// TrackFitMomentumRange Database OjbPtr
    DBObjPtr<TrackFitMomentumRange> m_trackFitMomentumRange;

    StoreArray<RecoTrack> m_RecoTracks; /**< RecoTracks StoreArray */

  };
}
