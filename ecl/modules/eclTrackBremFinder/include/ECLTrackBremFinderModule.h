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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>

namespace Belle2 {
  class ECLCluster;
  class Track;
  class EventMetaData;
  class RecoTrack;
  class BremHit;

  /**
   * Module to assign ECL Clusters resulting from Bremsstrahlung to the
   * primary electron track.
   */
  class ECLTrackBremFinderModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLTrackBremFinderModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLTrackBremFinderModule() = default;

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

  private:
    /** StoreArray ECLCluster */
    StoreArray<ECLCluster> m_eclClusters;
    /** StoreArray Track */
    StoreArray<Track> m_tracks;
    /** StoreArray RecoTrack */
    StoreArray<RecoTrack> m_recoTracks;
    /** StoreArray BremHits */
    StoreArray<BremHit> m_bremHits;
    /** StoreObjPtr EventMetaData */
    StoreObjPtr<EventMetaData> m_evtPtr;
    /**
     * Factor which is multiplied onto the cluster position error to check for matches
     */
    double m_clusterAcceptanceFactor = 3.0f;

    /**
     * Radii where virtual hits for the extrapolation will be generated
     * The default values are taken from bremsstrahlung studies
     * They represent the edge of the beampipe, the outer SVD wall and the inner CDC wall
     */
    std::vector<float> m_virtualHitRadii = {1.05, 16.0};

    /**
     * Fraction of the tracks energy the ECL cluster has to possess to be considered for bremsstrahlung finding
     */
    double m_relativeClusterEnergy = 0.02f;

    /**
     * Minimal/Maximal number of CDC hits, the track has to possess to be considered for bremsstrahlung finding
     */
    std::tuple<unsigned int, unsigned int> m_requestedNumberOfCDCHits = {1, 100};

    /**
     * Cut on the electron probability (from pid) of track
     */
    float m_electronProbabilityCut = 0;

    /**
     * Cut on the distance between the cluster position angle and the extrapolation angle
     */
    float m_clusterDistanceCut = 0.05;

    /**
     * StoreArray name of the ECLClusters for brem matching
     */
    std::string m_param_eclClustersStoreArrayName = "";

    /**
     * StoreArray name of the Tracks for brem matching
     */
    std::string m_param_tracksStoreArrayName = "";

    /**
     * StoreArray name of the RecoTracks for brem matching
     */
    std::string m_param_recoTracksStoreArrayName = "";
  };

} //Belle2
