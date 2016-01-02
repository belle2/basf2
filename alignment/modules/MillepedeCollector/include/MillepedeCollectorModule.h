/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MILLEPEDECOLLECTORMODULE_H
#define MILLEPEDECOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>
#include <genfit/Track.h>
#include <genfit/StateOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  /**
   * Calibration data collector for Millepede Algorithm
   *
   * Collects data from GBL-fitted tracks and produces binary files for Millepede
   *
   */
  class MillepedeCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MillepedeCollectorModule();

    /** Prepration */
    virtual void prepare();

    /** Data collection */
    virtual void collect();

    /** Only for closing mille binaries after each run */
    virtual void endRun();

    /** Make a name for mille binary (encodes module name + starting exp, run and event + process id) */
    std::string getUniqueMilleName();

    /** Get all useable daughter tracks for mother particle */
    std::vector<genfit::Track*> getParticlesTracks(std::vector<Particle*> particles);

    /** Compute the transformation matrix d(q/p,u',v',u,v)/d(x,y,z,px,py,pz) from state at first track point (vertex) */
    TMatrixD getGlobalToLocalTransform(genfit::MeasuredStateOnPlane msop);

  private:
    std::vector<std::string> m_tracks;  /**< Names of arrays with single genfit::Tracks fitted by GBL */
    std::vector<std::string>
    m_vertices;  /**< Name of particle list with mothers of daughters to be used with vertex constraint in calibration */
    std::vector<std::string>
    m_primaryVertices;  /**< Name of particle list with mothers of daughters to be used with vertex + IP profile constraint in calibration */
    bool m_doublePrecision;  /**< Use double (instead of single/float) precision for binary files */

  };
}

#endif /* MILLEPEDECOLLECTORMODULE_H */
