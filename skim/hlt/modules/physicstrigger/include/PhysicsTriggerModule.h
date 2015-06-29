/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/core/Module.h>

#include <string>
#include <memory>

namespace Belle2 {
  /**
   * physics trigger
   *
   *    *
   */
  class PhysicsTriggerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PhysicsTriggerModule();

    /** Destructor */
    virtual ~PhysicsTriggerModule();

    /** Initialize the parameters */
    virtual void initialize();

    /** Called when entering a new run. */
    virtual void beginRun();

    /** Event processor. */
    virtual void event();

    /** End-of-run action. */
    virtual void endRun();

    /** Termination action. */
    virtual void terminate();

    /**Initialize the parameters per event */
    void  initializeForEvent();

    /**Perform the selection criteria on event*/
    void eventSelect();

    /**
      Collect the information of event from VariableManager,
      and store them in StoreArray<PhysicsTriggerInformation>
     */
    void eventInformation();

    /**Access the selection criteria from xml file with Gearbox*/
    void ReadParameters();

    /**Classify the event with the slection criteria from xml file*/
    bool eventUserSelect();

  private:

    /**Control the condition of the physics trigger menu*/
    int m_parameterCutIndex;

    /**Switch of customing the selection criteria by user.*/
    int m_userCustomOpen;

    /**Access users' cut */
    std::string m_userCustomCut;

    /**Access users' cut*/
    std::unique_ptr<Variable::Cut> m_cut;

    /**hadron criteria*/

    /**the number of tracks*/
    int m_nTrackCutMin_had;
    /**the number of ECL clusters*/
    int m_nECLClusterCutMin_had;

    /**the total visible energy*/
    float m_EvisCutMin_had;
    /**the total deposit energy in ECL*/
    float m_EsumCutMin_had;
    /**the total deposit energy in ECL*/
    float m_EsumCutMax_had;

    /**tautau criteria*/

    /**the maximum number of tracks*/
    int m_nTrackCutMax_tau;
    /**the minimum number of tracks*/
    int m_nTrackCutMin_tau;
    /**the maximum ECL deposited energy*/
    float m_EsumCutMax_tau;
    //float m_AngleTTCut_tau; /**the angle of the 1st and 2nd highest momentum tracks in center-of-mass energy */

    /**bhabha criteria*/
    int m_nTrackCutMin_bhabha;/**the min number of tracks*/

    /**the min number of ECL clusters*/
    int m_nECLClusterCutMin_bhabha;

    /**the largest deposity energy cluster in ECL*/
    float m_ECLClusterE1CutMin_bhabha;

    /**the min deposited energy in ECL*/
    float m_EsumCutMin_bhabha;

    /**the largest angle between the charged tracks*/
    float m_AngleTTCutMax_bhabha;

    /**Mu-pair criteria*/

    /**the min number of tracks*/
    int m_nTrackCutMin_dimu;

    /**the min total deposited energy*/
    float m_EsumCutMin_dimu;

    /**the largest angle between the ECL clusters*/
    float m_AngleTTCutMax_dimu;

    /**Gamma-pair criteria*/

    /**the max max number of tracks*/
    int m_nTrackCutMax_digamma;

    /**the min number of ECL clusters */
    int m_nECLClusterCutMin_digamma;

    /**the min number of ECL clusters*/
    float m_ECLClusterE1CutMin_digamma;

    /**the min total ECL deposited energy*/
    float m_EsumCutMin_digamma;

    /**the angle between the 1st and 2nd highest cluster in center-of-mass energy*/
    float m_AngleGGCutMax_digamma;

    /**two photon criteria*/


    /**Hadronic: 0-dr, 1-dz*/
    //std::vector<float> m_parameterCutHadronic;
    //std::vector<float> m_parameterCutTautau;
    /**summarize the trigger results*/
    int m_summary;

    /**the number of good tracks*/
    int m_ngoodTracks;

    /**the number of good clusters in ECL*/
    int m_ngoodECLClusters;

    /**the number of goog clusters in KLM*/
    int m_nKLMClusters;

    /**the total visible energy*/
    float m_Evis;

    /**the absolote total momentum in z coodinate*/
    float m_Pzvis;

    /**the angle between two charged tracks*/
    float m_angleTT;

    /**the max angle between two charged tracks*/
    float m_maxangleTT;

    /**the angle between two clusters*/
    float m_angleGG;

    /**Dr*/
    float m_dr;

    /**Dz*/
    float m_dz;

    /**the transverse momentum*/
    float m_pt;

    /**the momentum*/
    float m_p;

    /**the cos polar angle*/
    float m_costheta;

    /**azimuthal angle*/
    float m_phi;

    /**the largest momentum*/
    float m_p1;

    /**the second largest momentum*/
    float m_p2;

    /**the track ID with the largest momentum*/
    int m_ip1;

    /**the track ID with the socond largest momentum*/
    int m_ip2;

    /**the most energetic cluster*/
    float m_e1;

    /**the second most energetic cluster*/
    float m_e2;

    /**the cluster ID with the most energetic cluster*/
    int m_ie1;

    /**the cluster ID with the second most energetic cluster*/
    int m_ie2;

    /**charge*/
    int m_Charge;

    /**the energy of ECL cluster*/
    float m_ECLClusterE;

    /**the polar angle of ECL cluster*/
    float m_ECLClusterTheta;

    /**the azimuthal angle of ECL cluster*/
    float m_ECLClusterPhi;

    /**the timing of ECL cluster*/
    float m_ECLClusterTiming;
    /**the energy sum of good ECL clusters*/
    float m_Esum;


  };
}
