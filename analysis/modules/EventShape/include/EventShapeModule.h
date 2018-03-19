/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Villanueva, Ami Rostomyan, Jorge Martinez         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTSHAPEMODULE_H
#define EVENTSHAPEMODULE_H

#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <TVector3.h>

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventShape.h>

#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ContinuumSuppression/Thrust.h>


namespace Belle2 {
  /**
   * Module to compute Thrust of a particle list, mainly used to compute the thrust of a tau-taubar event.
   *
   *    *
   */
  class EventShapeModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    EventShapeModule();

    /** free memory */
    virtual ~EventShapeModule();

    /** Define the physical parameters. look for them in database. */
    virtual void initialize();

    /** Define run parameters. */
    virtual void beginRun();

    /** Define event parameters */
    virtual void event();

    /** Finish the run. */
    virtual void endRun();

    /** finish the execution  */
    virtual void terminate();


  private:

    std::vector<std::string> m_particleLists;  /**< Name of the ParticleList */

    /** Get all momenta and then calculate the thrust for this event */
    std::vector<TVector3> particleMomentumList, particleMomentumListCMS;
    void getParticleMomentumLists(std::vector<std::string> particleLists);

    TVector3 getThrustOfEvent();
    TVector3 getMissingMomentumCMS();
    TVector3 getMissingMomentum();
  };
}

#endif /* EVENTSHAPEMODULE_H */
