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
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
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

  private:
    /**max value of dr to be identified as match*/
    double m_dr_threshold;
    /**max value of dz to be identified as match */
    double m_dz_threshold;
    /**the 2D finder track list*/
    std::string m_2d_tracklist;
    /**the 3D NN track list*/
    std::string m_3d_tracklist;
    /**the ecl cluster list*/
    std::string m_clusterlist;
    /**the distance in phi direction between track and cluster*/
    //double m_dr;
    /**the distance in z direction between track and cluster*/
    //double m_dz;
    /**the matched 2d track list*/
    std::string m_2dmatch_tracklist;
    /**the matched 3d track list*/
    std::string m_3dmatch_tracklist;

  };
}
