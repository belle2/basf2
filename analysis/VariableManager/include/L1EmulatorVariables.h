/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/ECLCluster.h>

namespace Belle2 {

  namespace Variable {

    /**
     * The variables used in LE are all defined based on the
     * the good charged tracks and the ECL clusters passing the selection
     * criteria
     */


    /**
     * return the number of the charged tracks
     */
    double nTracksLE(const Particle*);


    /**
     * return the number of the tracks matched to ECL Clusters
     */
    double nECLMatchTracksLE(const Particle*);


    /**
     * return the number of the tracks matched to KLM Clusters
     */
    double nKLMMatchTracksLE(const Particle*);

    /** trk1 is the track with the largest momentum*/
    /** trk2 is the track with the second largest momentum*/

    /**
     * return trk1's momentum
     */
    double P1BhabhaLE(const Particle*);

    /**
     * return trk2's momentum
     */
    double P2BhabhaLE(const Particle*);

    /**
     * return trk2's polar angle
     */
    double Theta2BhabhaLE(const Particle*);

    /**
     * return trk2's azimuthal angle
     */
    double Phi2BhabhaLE(const Particle*);


    /**
     * return trk1's polar angle
     */
    double Theta1BhabhaLE(const Particle*);

    /**
     * return trk1's azimuthal angle
     */
    double Phi1BhabhaLE(const Particle*);

    /**
     * return the energy of ECL cluster matched to trk1
     */
    double E1BhabhaLE(const Particle*);

    /**
     * return the energy of ECL cluster matched to trk2
     */
    double E2BhabhaLE(const Particle*);

    /**
     * return trk1's charge
     */
    double Charge1BhabhaLE(const Particle*);


    /**
     * return trk2's charge
     */
    double Charge2BhabhaLE(const Particle*);

    /**
     * return the maximum angle between tracks
     */
    double maxAngleTTLE(const Particle*);
    /**
     * return the number of the ECL clusters
     */
    double nClustersLE(const Particle*);

    /**
     * return the sum energy of the  ECL clusters
     */
    double EtotLE(const Particle*);

    /**
     * return the number of KLM Clusters
     */
    double nKLMClustersLE(const Particle*);


    /**C1 is the most energnetic ECL Cluster*/
    /**C2 is the second most energnetic ECL Cluster*/

    /**
     * return C1's energy
     */
    double EC1LE(const Particle*);


    /**
     * return C1's polar angle
     */
    double ThetaC1LE(const Particle*);

    /**
     * return C1's azimuthal angle
     */
    double PhiC1LE(const Particle*);

    /**
     * return the distance C1 and IP
     */
    double RC1LE(const Particle*);

    /**
     * return C2's energy
     */
    double EC2LE(const Particle*);

    /**
     * return C2's polar angle
     */
    double ThetaC2LE(const Particle*);

    /**
     * return C2's azimuthal angle
     */
    double PhiC2LE(const Particle*);

    /**
     * return the distance C2 and IP
     */
    double RC2LE(const Particle*);

    /**
     * return C1
     */
    const ECLCluster*  ECLClusterC1LE(const Particle*);

    /**
     * return C2
     */
    const ECLCluster*  ECLClusterC2LE(const Particle*) ;

    /**
     * return the angle between C1 and C2
     */
    double AngleGGLE(const Particle*);

    /**
     * return trk1
     */
    const Particle* T1(const Particle*);

    /**
    * return trk2
    */
    const Particle* T2(const Particle*);

    /**
    * return ecl Bhabha veto result
    */
    double eclBhabha(const Particle*);

    /**
    * return Bhabha veto result
    */
    double BhabhaVeto(const Particle*);

    /**
    * return gg veto result
    */
    double ggVeto(const Particle*);

  }

}
