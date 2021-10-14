/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns the KlId associated to the ECL cluster
     */
    double eclClusterKlId(const Particle* particle);

    /**
     * return digit level MVA that uses pulse shape discrimination to identify hadronic vs electromagnetic showers.
     */
    double eclPulseShapeDiscriminationMVA(const Particle* particle);

    /**
     * return MVA output that uses shower-related variables to distinguish between true photon and hadronic splitoff clusters
     */
    double hadronicSplitOffSuppression(const Particle* particle);

    /**
     * return MVA output that uses shower-related variables to distinguish between true photon and beam background clusters
     */
    double beamBackgroundSuppression(const Particle* particle);

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
     * return array index of the nearest track hitting the ECL
     * Note: This distance is calculated on the reconstructed level
     */
    double eclClusterIsolationID(const Particle* particle);

    /**
     * return variable value of the nearest track hitting the ECL
     * Note: This distance is calculated on the reconstructed level
     */
    Manager::FunctionPtr eclClusterIsolationVar(const std::vector<std::string>& arguments);

    /**
     * return DeltaL for the shower shape
     * Note: This distance is calculated on the reconstructed level and is temporarily
     * included to the ECLCLuster MDST data format for studying purposes. If it is found
     * to be not crucial for physics analysis then this variable will be removed in future releases.
     * Therefore, keep in mind that this variable might be removed in the future!
     */
    double eclClusterDeltaL(const Particle* particle);

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

    /**
     * return whether or not ECL cluster's timing fit failed
     */
    double eclClusterHasFailedTiming(const Particle* particle);

    /**
     * return  Error on clusters phi position
     */
    double eclClusterErrorPhi(const Particle* particle);

    /**
     * return  Error on clusters theta position
     */
    double eclClusterErrorTheta(const Particle* particle);

    /**
     * return ECL cluster's Error on timing information
     */
    double eclClusterErrorTiming(const Particle* particle);

    /**
     * return whether or not calculation of ECL cluster's error on the timing failed
     */
    double eclClusterHasFailedErrorTiming(const Particle* particle);

    /**
     * return the energy of the crystal with highest energy
     */
    double eclClusterHighestE(const Particle* particle);

    /**
     * return the cellID [1,8736] of the crystal with highest energy
     */
    double eclClusterCellId(const Particle* particle);

    /**
     * return the thetaID [0,68] of the crystal with highest energy
     */
    double eclClusterThetaId(const Particle* particle);

    /**
     * return the phiID [0,143] of the crystal with highest energy
     */
    double eclClusterPhiId(const Particle* particle);

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
    double eclClusterAbsZernikeMoment51(const Particle* particle);

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
     * [Expert] The invariant mass calculated from all ECLCluster daughters (i.e. photons) and cluster-matched tracks using the CLUSTER 4-MOMENTA.
     * Used for ECL-based dark sector physics and debugging track-cluster matching.
     */
    double eclClusterOnlyInvariantMass(const Particle* part);

    /**
     * Returns cluster mdst array index
     */
    double eclClusterMdstIndex(const Particle* particle);

    /**
     * Returns function which returns true if the connected region of the particle's cluster is shared by another cluster.
     * This other cluster can be neutral or matched to a track.
     * A cut on the properties of the other cluster can be provided.
     */
    Manager::FunctionPtr photonHasOverlap(const std::vector<std::string>& arguments);

  }
} // Belle2 namespace
