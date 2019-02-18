/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *               Alon Hershenhorn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * return digit level MVA that uses pulse shape discrimination to identify hadronic vs electromagnetic showers.
     */
    double eclPulseShapeDiscriminationMVA(const Particle* particle);

    /**
     * returns the weighted sum of digits in cluster with significant scintillation emission (> 3 MeV) in the hadronic scintillation component
     * Variable is used to separate hadronic showers from electromagnetic showers
     */
    double eclClusterNumberOfHadronDigits(const Particle* particle);

    /**
     * return 1/2/3 if the ECL Cluster is detected in the forward/barrel/backward region
     * return 11 and 13 if the ECL Cluster is in gap between barrel and forward respectively backward
     * return 0 otherwise
     */
    double eclClusterDetectionRegion(const Particle* particle);

    /**
     * return distance from eclCluster to nearest track hitting the ECLCluster
     * Note: This distance is calculated on the reconstructed level
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
     * [Legacy] Returns true if the cluster with given attributes passes the Belle 'good gamma' criteria.
     */
    bool isGoodBelleGamma(int region, double energy);

    /**
     * [Legacy]
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 100 MeV
     * Barrel   : E >  50 MeV
     * Backward : E > 150 MeV
     */
    double goodBelleGamma(const Particle* particle);

    /**
     * return ECL cluster's Error on Energy
     */
    double eclClusterErrorE(const Particle* particle);

    /**
     * return ECL cluster's corrected energy
     */
    double eclClusterE(const Particle* particle);

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


    /** return  Error on clusters phi position  */
    double eclClusterErrorPhi(const Particle* particle);

    /** return  Error on clusters theta position  */
    double eclClusterErrorTheta(const Particle* particle);

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
     *
     * return number of hits associated to this cluster
     */
    double eclClusterNHits(const Particle* particle);

    /**
     * return 1/0 if charged track is/is not Matched to this cluster
     */
    double eclClusterTrackMatched(const Particle* particle);

    /**
     * return the number of tracks matched to the ECLCluster from this particle,
     * will be zero for neutrals, but can be one or more in some cases where
     * several tracks were extrapolated into the same cluster. NAN if the particle
     * has no ECLCluster associated.
     */
    double nECLClusterTrackMatches(const Particle* particle);

    /**
     * return the ConnectedRegion ID of this cluster
     */
    double eclClusterConnectedRegionId(const Particle* particle);

    /**
     * return the Cluster ID within the connected region of this cluster
     */
    double eclClusterId(const Particle* particle);

    /**
     * [deprecated] return the Hypothesis ID of this cluster
     */
    double eclClusterHypothesisId(const Particle* particle);

    /**
     * Returns 1.0 if the cluster has the 'N photons' hypothesis (historically
     * called 'N1'), 0.0 if not, and NaN if no cluster is associated to the
     * particle.
     */
    double eclClusterHasNPhotonsHypothesis(const Particle* particle);

    /**
     * Returns 1.0 if the cluster has the 'neutral hadrons' hypothesis
     * (historically called 'N2'), 0.0 if not, and NaN if no cluster is
     * associated to the particle.
     */
    double eclClusterHasNeutralHadronHypothesis(const Particle* particle);

    /**
     * return 1 if cluster has pulse shape discrimination variables computed
     */
    double eclClusterHasPulseShapeDiscrimination(const Particle* particle);

    /**
     * return 1.0 if cluster matched to a trigger cluster
     */
    double eclClusterTrigger(const Particle* particle);

    /**
     * return theta of extrapolated track
     */
    double eclExtTheta(const Particle* particle);

    /**
     * return phi of extrapolated track
     */
    double eclExtPhi(const Particle* particle);

    /**
     * return phi id of extrapolated track
     */
    double eclExtPhiId(const Particle* particle);

    /**
     * return three digit energy sum FWD barrel
     */
    double eclEnergy3FWDBarrel(const Particle* particle);

    /**
     * return three digit energy sum FWD endcap
     */
    double eclEnergy3FWDEndcap(const Particle* particle);

    /**
     * return three digit energy sum BWD barrel
     */
    double eclEnergy3BWDBarrel(const Particle* particle);

    /**
     * return three digit energy sum BWD endcap
     */
    double eclEnergy3BWDEndcap(const Particle* particle);

    /**
     * Returns the ECL weighted average time of the daughters of the provided particle
     */
    double weightedAverageECLTime(const Particle* particle);

    /**
     * Returns the maximum weighted distance between the individual ECL time and the ECL average time of all daughters
     */
    double maxWeightedDistanceFromAverageECLTime(const Particle* particle);

    /**
     * return the number of crystals (ECLCalDigits) that are out of time in the FWD endcap
     */
    double nECLOutOfTimeCrystalsFWDEndcap(const Particle*);

    /**
     * return the number of crystals (ECLCalDigits) that are out of time in the barrel
     */
    double nECLOutOfTimeCrystalsBarrel(const Particle*);

    /**
     * return the number of crystals (ECLCalDigits) that are out of time in the BWD endcap
     */
    double nECLOutOfTimeCrystalsBWDEndcap(const Particle*);

    /**
     * return the number of crystals (ECLCalDigits) that are out of time
     */
    double nECLOutOfTimeCrystals(const Particle*);

    /**
     * return the number of showers in the ECL that do not become clusters from
     * the forward endcap
     */
    double nRejectedECLShowersFWDEndcap(const Particle*);

    /**
     * return the number of showers in the ECL that do not become clusters from
     * the barrel
     */
    double nRejectedECLShowersBarrel(const Particle*);

    /**
     * return the number of showers in the ECL that do not become clusters from
     * the backward endcap
     */
    double nRejectedECLShowersBWDEndcap(const Particle*);

    /**
     * return the number of showers in the ECL that do not become clusters
     */
    double nRejectedECLShowers(const Particle*);

    /** energy over momentum can be used to separate electrons from muons*/
    double eclClusterEoP(const Particle* part);

    /**
     * return the number of TCs above threshold
     */
    double getNumberOfTCs(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC energy
     */
    double getEnergyTC(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC energy based on ECLCalDigits
     */
    double getEnergyTCECLCalDigit(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC timing
     */
    double getTimingTC(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC event timing
     */
    double getEvtTimingTC(const Particle*);

    /**
     * return the  TC Id with maximum FADC
     */
    double getMaximumTCId(const Particle*);

    /**
     * returns the TC hit window
     */
    double eclHitWindowTC(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC timing based on ECLCalDigits
     */
    double getTimingTCECLCalDigit(const Particle*, const std::vector<double>& vars);

    /**
    * return the TC energy sum
    */
    double eclEnergySumTC(const Particle*, const std::vector<double>& vars);

    /**
    * return the TC energy sum based on ECLCalDigits
    */
    double eclEnergySumTCECLCalDigit(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC energy sum based on ECLCalDigits that are in ECLClusters above threshold
     */
    double eclEnergySumTCECLCalDigitInECLCluster(const Particle*);

    /**
     * return the energy sum based on ECLCalDigits that are in ECLClusters above threshold
     */
    double eclEnergySumECLCalDigitInECLCluster(const Particle*);

    /**
    * return the threshold TC energy sum based on ECLCalDigits that are in ECLClusters
    */
    double eclEnergySumTCECLCalDigitInECLClusterThreshold(const Particle*);

    /**
     * Returns the number of TCs above threshold
     */
    double eclNumberOfTCsForCluster(const Particle* particle, const std::vector<double>& vars);

    /**
     * Returns the FADC sum above threshold for given hit windows
     */
    double eclTCFADCForCluster(const Particle* particle, const std::vector<double>& vars);

    /**
     * Returns true if cluster is related to maximum TC
     */
    double eclTCIsMaximumForCluster(const Particle* particle);

    /**
     * Returns cluster mdst array index
     */
    double eclMdstIndex(const Particle* particle);

  }
} // Belle2 namespace
