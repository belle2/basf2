/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PHYSICSTRIGGERMODULE_H
#define PHYSICSTRIGGERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>

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

    bool eventUserSelect();

  private:

    /**Control the condition of the physics trigger menu*/
    int m_parameterCutIndex;

    int m_userCustomOpen;

    Variable::Cut::Parameter m_userCustomCut;
    Variable::Cut m_cut;
    /**hadron criteria*/

    int m_nTrackCutMin_had;/**the number of tracks*/
    int m_nECLClusterCutMin_had; /**the number of ECL clusters*/
    float m_EvisCutMin_had; /**the total visible energy*/
    float m_EsumCutMin_had;/**the total deposit energy in ECL*/
    float m_EsumCutMax_had;/**the total deposit energy in ECL*/

    /**tautau criteria*/
    int m_nTrackCutMax_tau;
    int m_nTrackCutMin_tau;
    float m_EsumCutMax_tau;
    float m_AngleTTCut_tau; /**the angle of the 1st and 2nd highest momentum tracks in center-of-mass energy */

    /**bhabha criteria*/
    int m_nTrackCutMin_bhabha;
    int m_nECLClusterCutMin_bhabha;
    float m_ECLClusterE1CutMin_bhabha;/**the largest deposity energy cluster in ECL*/
    float m_EsumCutMin_bhabha;
    float m_AngleTTCutMax_bhabha;

    /**Mu-pair criteria*/
    int m_nTrackCutMin_dimu;
    float m_EsumCutMin_dimu;
    float m_AngleTTCutMax_dimu;

    /**Gamma-pair criteria*/
    int m_nTrackCutMax_digamma;
    int m_nECLClusterCutMin_digamma;
    float m_ECLClusterE1CutMin_digamma;
    float m_EsumCutMin_digamma;
    float m_AngleGGCutMax_digamma;/**the angle between the 1st and 2nd highest cluster in center-of-mass energy*/

    /**two photon criteria*/


    /**Hadronic: 0-dr, 1-dz*/
    //std::vector<float> m_parameterCutHadronic;
    //std::vector<float> m_parameterCutTautau;

    int m_summary;
    int m_ngoodTracks;
    int m_ngoodECLClusters;
    float m_Evis;
    float m_Pzvis;
    float m_angleTT;
    float m_angleGG;
    float m_dr;
    float m_dz;
    float m_pt;
    float m_p;
    float m_costheta;
    float m_phi;
    float m_p1;
    float m_p2;
    int m_ip1;
    int m_ip2;
    int m_Charge;
    float m_ECLClusterE;
    float m_ECLClusterTheta;
    float m_ECLClusterPhi;
    float m_ECLClusterTiming;

    float m_Esum; /**the energy sum of good ECL clusters*/
    float m_e1; /**the most energitic cluster*/
    float m_e2; /**the most energitic cluster*/
    int m_ie1;
    int m_ie2;


  };
}

#endif /* PHYSICSTRIGGERMODULE_H */
