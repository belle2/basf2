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
     * The variables used in LE and HLT are all defined based on the
     * the good charged tracks and the ECL clusters passing the selection
     * criteria
     */


    /**
     * return the number of the charged tracks
     */
    double nTracksLE(const Particle*);

    /**
     * return the number of the charged long tracks
     */
    double nLongTracksLE(const Particle*);

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
     * return trk1's momentum in Lab
     */
    double P1BhabhaLE(const Particle*);

    /**
     * return trk1's momentum in CMS
     */
    double P1CMSBhabhaLE(const Particle*);

    /**
     * return trk2's momentum in Lab
     */
    double P2BhabhaLE(const Particle*);

    /**
     * return trk2's momentum in CMS
     */
    double P2CMSBhabhaLE(const Particle*);

    /**
     * return the total momentum of trk1 and trk2 in CMS
     */
    double P12CMSBhabhaLE(const Particle*);


    /**
     * return trk1's E/p
     */
    double EoPT1BhabhaLE(const Particle*);

    /**
     * return trk2's E/p
     */
    double EoPT2BhabhaLE(const Particle*);

    /**
     * return trk1's momentum
     */
    double Pt1BhabhaLE(const Particle*);

    /**
     * return trk2's momentum
     */
    double Pt2BhabhaLE(const Particle*);
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
     * return the layers of KLM matched to trk1
     */
    double Layer1BhabhaLE(const Particle*);

    /**
     * return the layers of KLM matched to trk2
     */
    double Layer2BhabhaLE(const Particle*);

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
     * return the maximum angle between KLM clusters
     */
    double maxAngleMMLE(const Particle*);

    /**
     * return the maximum angle between CDC tracks and KLM clusters
     */
    double maxAngleTMLE(const Particle*);

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

    /**
     *return the largest layers of the KLM Clusters
     */
    double LayerKLMCluster1LE(const Particle*);

    /**
     *return the second largest layers of the KLM Clusters
     */
    double LayerKLMCluster2LE(const Particle*);

    /**C1 is the most energnetic ECL Cluster*/
    /**C2 is the second most energnetic ECL Cluster*/

    /**
     * return C1's energy
     */
    double EC1LE(const Particle*);

    /**
     * return C1's energy in CMS
     */
    double EC1CMSLE(const Particle*);


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
     * return C2's energy in CMS
     */
    double EC2CMSLE(const Particle*);

    /**
     * return the sum of C1 and C2's energies in CMS
     */
    double EC12CMSLE(const Particle*);

    /**
     * return C1 + C2 energy
     */
    double EC12LE(const Particle*);

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
     * return ENeutral
     */
    const ECLCluster*  ECLClusterNeutralLE(const Particle*);

    /**
     * return ThetaNeutral
     */
    double ThetaNeutralLE(const Particle*);

    /**
     * return PhiNeutral
     */
    double PhiNeutralLE(const Particle*);

    /**
     * return ENeutral
     */
    double ENeutralLE(const Particle*);

    /**
     * return RNeutral
     */
    double RNeutralLE(const Particle*);

    /**
     * return C2
     */
    const ECLCluster*  ECLClusterC2LE(const Particle*) ;

    /**
     * return the angle between C1 and C2
     */
    double AngleGGLE(const Particle*);

    /**
     * return the max angle between C1 (C2) and T1 (T2)
     */
    double AngleGTLE(const Particle*);

    /**
     * return trk1
     */
    const Particle* T1(const Particle*);

    /**
    * return trk2
    */
    const Particle* T2(const Particle*);


    /**
    * return the polar angle of track with negative charge
    */
    double MinusThetaBhabhaLE(const Particle*);

    /**
    * return the number of tracks satified Eid
    */
    double nEidLE(const Particle*);

    /**
     * return the visible energy defined as the sum
     * of the track momenta and the ECL cluster energies
     */
    double VisibleEnergyLE(const Particle*);

    /**
     * return the sum of the absolute Pz of the total visible energy
     */
    double VisiblePzLE(const Particle*);



  }

}
