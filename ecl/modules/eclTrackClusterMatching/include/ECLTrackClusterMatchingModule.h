/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TF1.h>

#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>

namespace Belle2 {
  class ECLTrackClusterMatchingParameterizations;
  class ECLTrackClusterMatchingThresholds;

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
    virtual void initialize() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    /** Clean up anything created in initialize(). */
    virtual void terminate() override;

  private:
    /** Check if status of extrapolated hit is entering of ECL. */
    bool isECLEnterHit(const ExtHit& extHit) const;

    /** Check if extrapolated hit is inside ECL and matches one of the desired categories. */
    bool isECLHit(const ExtHit& extHit) const;

    /** Calculate matching quality based on phi and theta consistencies */
    double showerQuality(double deltaPhi, double deltaTheta, double pt, int eclDetectorRegion, int hitStatus) const;

    /** Calculate phi consistency based on difference in azimuthal angle.
     *
     *  Parametrization depends on transverse momentum and detector region of cluster.
     */
    double phiConsistency(double deltaPhi, double pt, int eclDetectorRegion, int hitStatus) const;

    /** Calculate theta consistency based on difference in polar angle.
     *
     *  Parametrization depends on transverse momentum and detector region of cluster.
     */
    double thetaConsistency(double deltaTheta, double pt, int eclDetectorRegion, int hitStatus) const;

    bool trackTowardsGap(double theta) const; /**< return if track points towards gap or adjacent part of barrel */

    void optimizedPTMatchingConsistency(double theta, double pt); /**< choose criterion depending on track's pt */

    StoreArray<ExtHit> m_extHits; /**< Required input array of ExtHits */
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required input array of TrackFitResults */
    StoreArray<ECLCluster> m_eclClusters; /**< Required input array of ECLClusters */
    StoreArray<ECLShower> m_eclShowers; /**< Required input array of ECLShowers */
    StoreArray<ECLCalDigit> m_eclCalDigits; /**< Required input array of ECLCalDigits */

    DBObjPtr<ECLTrackClusterMatchingParameterizations> m_matchingParameterizations; /**< Parameterizations of RMS */

    TF1 f_phiRMSFWDCROSS; /**< function to describe phi RMS for FWD CROSS */
    TF1 f_phiRMSFWDDL; /**< function to describe phi RMS for FWD DL */
    TF1 f_phiRMSFWDNEAR; /**< function to describe phi RMS for FWD NEAR */
    TF1 f_phiRMSBRLCROSS; /**< function to describe phi RMS for BRL CROSS */
    TF1 f_phiRMSBRLDL; /**< function to describe phi RMS for BRL DL */
    TF1 f_phiRMSBRLNEAR; /**< function to describe phi RMS for BRL NEAR */
    TF1 f_phiRMSBWDCROSS; /**< function to describe phi RMS for BWD CROSS */
    TF1 f_phiRMSBWDDL; /**< function to describe phi RMS for BWD DL */
    TF1 f_phiRMSBWDNEAR; /**< function to describe phi RMS for BWD NEAR */
    TF1 f_thetaRMSFWDCROSS; /**< function to describe theta RMS for FWD CROSS */
    TF1 f_thetaRMSFWDDL; /**< function to describe theta RMS for FWD DL */
    TF1 f_thetaRMSFWDNEAR; /**< function to describe theta RMS for FWD NEAR */
    TF1 f_thetaRMSBRLCROSS; /**< function to describe theta RMS for BRL CROSS */
    TF1 f_thetaRMSBRLDL; /**< function to describe theta RMS for BRL DL */
    TF1 f_thetaRMSBRLNEAR; /**< function to describe theta RMS for BRL NEAR */
    TF1 f_thetaRMSBWDCROSS; /**< function to describe theta RMS for BWD CROSS */
    TF1 f_thetaRMSBWDDL; /**< function to describe theta RMS for BWD DL */
    TF1 f_thetaRMSBWDNEAR; /**< function to describe theta RMS for BWD NEAR */

    DBObjPtr<ECLTrackClusterMatchingThresholds> m_matchingThresholds; /**< Optimized matching thresholds */

    std::vector<std::pair<double, double>> m_matchingThresholdValuesFWD; /**< Matching threshold values for FWD */
    std::vector<std::pair<double, std::pair<double, double>>> m_matchingThresholdValuesBRL; /**< Matching threshold values for BRL */
    std::vector<std::pair<double, double>> m_matchingThresholdValuesBWD; /**< Matching threshold values for BWD */

    /** members of ECLTrackClusterMatching Module */
    /** Track cluster matching method
     *
     *  true: track cluster matching based on angular distance between track at specific points and cluster centers
     *  false: track cluster matching based on tracks entering crystals belonging to cluster
     */
    bool m_angularDistanceMatching;
    bool m_useOptimizedMatchingConsistency; /**< if true, a theta dependent matching criterion will be used */
    bool m_skipZeroChargeTracks; /**< if true, tracks whose charge has been set to zero are excluded from track-cluster matching */
    double m_matchingConsistency; /**< minimal quality of ExtHit-ECLCluster pair for positive track-cluster match */
    double m_matchingPTThreshold; /**< pt limit between angular-distance based and crystal-entering based matching algorithm */
    double m_brlEdgeTheta; /**< distance of polar angle from gaps where crystal-entering based matching is applied (in rad) */
    int m_minimalCDCHits; /**< minimal required number of CDC hits before track-cluster match is initiated */
  };
} //Belle2
