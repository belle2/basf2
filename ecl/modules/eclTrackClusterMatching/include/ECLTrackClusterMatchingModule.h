/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>

namespace Belle2 {

  /** The module creates and saves a Relation between Tracks and ECLCluster in
   *  the DataStore. It uses the existing Relation between Tracks and ExtHit as well
   *  as the Relation between ECLCluster and ExtHit.
   */
  class ECLTrackClusterMatchingModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLTrackClusterMatchingModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLTrackClusterMatchingModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();

  private:

    /** Check if extrapolated hit is inside ECL and matches one of the desired categories. */
    bool isECLHit(const ExtHit& extHit) const;

    /** Calculate matching quality based on phi and theta consistencies */
    double clusterQuality(double deltaPhi, double deltaTheta, double transverseMomentum, int eclDetectorRegion) const;

    /** Calculate phi consistency based on difference in azimuthal angle.
     *
     *  Parametrization depends on transverse momentum and detector region.
     */
    double phiConsistency(double deltaPhi, double transverseMomentum, int eclDetectorRegion) const;

    /** Calculate theta consistency based on difference in polar angle.
     *
     *  Parametrization depends on transverse momentum and detector region.
     */
    double thetaConsistency(double deltaTheta, double transverseMomentum, int eclDetectorRegion) const;

    int getDetectorRegion(double theta) const; /**< return detector region based on polar angle */

    StoreArray<ExtHit> m_extHits; /**< Required input array of ExtHits */
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required input array of TrackFitResults */
    StoreArray<ECLCluster> m_eclClusters; /**< Required input array of ECLClusters */

    /** members of ECLTrackClusterMatching Module */
    bool m_useOptimizedMatchingConsistency; /**< if true, a theta dependent matching criterion will be used */
    double m_matchingConsistency; /**< minimal quality of ExtHit-ECLCluster pair for positive track-cluster match */
  };
} //Belle2
