/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI, Yun-Tsung Lai, Junhao Yin                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/core/Module.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/klm/dataobjects/KLMTriggerTrack.h>
#include <string>
#include <memory>

namespace Belle2 {
  /**
   * Match between CDC trigger track and ECL trigger cluster
   *
   *    *
   */
  class TRGGRLMatchModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TRGGRLMatchModule();

    /** Destructor */
    virtual ~TRGGRLMatchModule();

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

    /**calculate dr and dz between track and cluster*/
    void calculationdistance(CDCTriggerTrack* track, TRGECLCluster* cluster, double* ds, int _match3D);

    /**calculate dphi_d between track and cluster*/
    void calculationphiangle(CDCTriggerTrack* track, TRGECLCluster* cluster, int& dphi_d);

    /**calculate dphi between 2D track and KLM track*/
    void sectormatching_klm(CDCTriggerTrack* track, KLMTriggerTrack* klmtrack, double& dphi);

  private:

    /// Mode for TRGGRL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int m_simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int m_fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int m_firmwareSimulationMode;

    /**max value of dr to be identified as match*/
    double m_dr_threshold;
    /**max value of dz to be identified as match */
    double m_dz_threshold;
    /**max value of dphi_d to be identified as match, 1 digit = 10 degrees */
    int m_dphi_d_threshold;
    /**max value of dphi (CDC track to KLM sector) to be identified as match (in degrees) */
    double m_dphi_klm_threshold;
    /**the 2D finder track list*/
    std::string m_2d_tracklist;
    /**the 3D NN track list*/
    std::string m_3d_tracklist;
    /**the ecl cluster list*/
    std::string m_clusterlist;
    /**the KLM track list*/
    std::string m_klmtracklist;
    /**the distance in phi direction between track and cluster*/
    //double m_dr;
    /**the distance in z direction between track and cluster*/
    //double m_dz;
    /**the matched 2d track list*/
    std::string m_2dmatch_tracklist;
    /**the matched 2d track list by phi matching*/
    std::string m_phimatch_tracklist;
    /**the matched 3d track list*/
    std::string m_3dmatch_tracklist;
    /**the matched 2d track list by KLM matching*/
    std::string m_klmmatch_tracklist;

  };
}
