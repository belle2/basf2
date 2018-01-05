/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alon Hershenhorn (contents taken from Variables.h file)  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * return 1/2/3 if the ECL Cluster is detected in the forward/barrel/backward region
     * return 11 and 13 if the ECL Cluster is in gap between barrel and forward respectively backward
     * return 0 otherwise
     */
    double eclClusterDetectionRegion(const Particle* particle);

    /**
     * return distance from eclCluster to nearest track hitting the ECLCluster
     * Note: This distance is calculated on the reconstructed level and is temporarily
     * included to the ECLCLuster MDST data format for studying purposes. If it is found
     * to be effectively replaced by the 'minCluster2HelixDistance', which can be caluclated
     * on the analysis level then this variable will be removed in future releases.
     * Therefore, keep in mind that this variable might be removed in the future!
     */
    double eclClusterIsolation(const Particle* particle);

    /**
     * return DeltaL for the shower shape
     * Note: This distance is calculated on the reconstructed level and is temporarily
     * included to the ECLCLuster MDST data format for studying purposes. If it is found
     * to be not crutial for physics analysis then this variable will be removed in future releases.
     * Therefore, keep in mind that this variable might be removed in the future!
     */
    double eclClusterDeltaL(const Particle* particle);

    /**
     * return distance from eclCluster to nearest point on nearest Helix at the ECL cylindrical radius
     */
    double minCluster2HelixDistance(const Particle* particle);

    /**
     * Returns true if the cluster with given attributes passes 'good gamma' criteria.
     * @param calibrated set to false for goodGammaUncalibrated().
     */
    bool isGoodGamma(int region, double energy, bool calibrated);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 100 MeV
     * Barrel   : E >  90 MeV
     * Backward : E > 160 MeV
     */
    double goodGamma(const Particle* particle);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E_uncalib > 125 MeV && E9/E25>0.7
     * Barrel   : E_uncalib > 100 MeV
     * Backward : E_uncalib > 150 MeV
     */
    double goodGammaUncalibrated(const Particle* particle);

    /**
     * Returns true if the cluster with given attributes passes the Belle 'good gamma' criteria.
     */
    bool isGoodBelleGamma(int region, double energy);

    /**
     * Returns true if the cluster with given attributes passes loose skim 'good gamma' criteria.
     */
    bool isGoodSkimGamma(int region, double energy);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 100 MeV
     * Barrel   : E >  50 MeV
     * Backward : E > 150 MeV
     */
    double goodBelleGamma(const Particle* particle);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 30 MeV
     * Barrel   : E > 20 MeV
     * Backward : E > 40 MeV
     */
    double goodSkimGamma(const Particle* particle);

    /**
     * return ECL cluster's Error on Energy
     */
    double eclClusterErrorE(const Particle* particle);

    /**
     * return ECL cluster's uncorrected energy
     */
    double eclClusterUncorrectedE(const Particle* particle);

    /**
     * return ECL cluster's distance
     */
    double eclClusterR(const Particle* particle);

    /**
     * return ECL cluster's azimuthal angle
     */
    double eclClusterPhi(const Particle* particle);

    /**
     * return ECL cluster's polar angle
     */
    double eclClusterTheta(const Particle* particle);

    /**
     * return ECL cluster's timing
     */
    double eclClusterTiming(const Particle* particle);

    /**
     * return ECL cluster's Error on timing information
     */
    double eclClusterErrorTiming(const Particle* particle);

    /**
     * return the energy of the crystall with highest  energy
     */
    double eclClusterHighestE(const Particle* particle);

    /**
     * return ratio of energies of the central crystal and 3x3 crystals around the central crystal
     */
    double eclClusterE1E9(const Particle* particle);

    /**
     * return ratio of energies in inner 3x3 and (5x5 cells without corners)
     */
    double eclClusterE9E21(const Particle* particle);

    /**
     * Deprecated - kept for backwards compatibility
     */
    inline double eclClusterE9E25(const Particle* particle) {return eclClusterE9E21(particle);}

    /**
     * return absolute value of Zernike Moment 40
     */
    double eclClusterAbsZernikeMoment40(const Particle* particle);

    /**
     * return absolute value of Zernike Moment 51
     */
    double eclClusterAbsZernikeMoment40(const Particle* particle);

    /**
     * return result of MVA using zernike moments
     */
    double eclClusterZernikeMVA(const Particle* particle);

    /**
     * return second moment shower shape variable
     */
    double eclClusterSecondMoment(const Particle* particle);

    /**
     * return LAT (shower variable)
     */
    double eclClusterLAT(const Particle* particle);

    /**
     * return high momentum pi0 likelihood.
     */
    double eclClusterMergedPi0(const Particle* particle);

    /**
     *
     * return number of hits associated to this cluster
     */
    double eclClusterNHits(const Particle* particle);

    /**
     * return 1/0 if charged track is/is not Matched to this cluster
     */
    double eclClusterTrackMatched(const Particle* particle);

    /**
     * return the ConnectedRegion ID of this cluster
     */
    double eclClusterConnectedRegionId(const Particle* particle);

    /**
     * return the Cluster ID of this cluster
     */
    double eclClusterId(const Particle* particle);

    /**
     * return the Hypothesis ID of this cluster
     */
    double eclClusterHypothesisId(const Particle* particle);

  }
} // Belle2 namespace
