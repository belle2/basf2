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
   * Module to compute event shape variables such as thrust, missing energy and mass2, visible energy, etc.
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

    /** A vector of the particles' 4-momenta in lab*/
    std::vector<TLorentzVector> m_particleMomentumList;
    /** A vector of the particles' 4-momenta in the CMS */
    std::vector<TLorentzVector> m_particleMomentumListCMS;
    /** Fill the lists of particles' momenta */
    void getParticleMomentumLists(std::vector<std::string> particleLists);

    /** Calculate the thrust for this event */
    TVector3 getThrustOfEvent();
    /** Calculate the missing momentum in the lab system for this event */
    TVector3 getMissingMomentum();
    /** Calculate the missing momentum in the CMS for this event */
    TVector3 getMissingMomentumCMS();
    /** Calculate the missing energy in the CMS for this event */
    float getMissingEnergyCMS();
    /** Calculate the visible energy for this event */
    float getVisibleEnergy();
  };
}

#endif /* EVENTSHAPEMODULE_H */
