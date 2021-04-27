/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Villanueva, Ami Rostomyan, Jorge Martinez         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TVector3.h>

#include <framework/core/Module.h>

#include <analysis/dataobjects/EventKinematics.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /**
   * Module to compute global quantities related to the event kinematics, like total missing energy and mass2, visible energy, etc.
   */
  class EventKinematicsModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    EventKinematicsModule();

    /** free memory */
    virtual ~EventKinematicsModule();

    /** Define the physical parameters. look for them in database. */
    virtual void initialize() override;

    /** Define event parameters */
    virtual void event() override;

    /** finish the execution  */
    virtual void terminate() override;


  private:

    StoreObjPtr<EventKinematics> m_eventKinematics; /**< event kinematics object pointer */

    bool m_usingMC;  /**< Is from MC */

    std::vector<std::string> m_particleLists;  /**< Name of the ParticleList */

    /** A vector of the particles' 4-momenta in lab*/
    std::vector<TLorentzVector> m_particleMomentumList;
    /** A vector of the particles' 4-momenta in the CMS */
    std::vector<TLorentzVector> m_particleMomentumListCMS;
    /** A vector of the photons' 4-momenta in the lab */
    std::vector<TLorentzVector> m_photonsMomentumList;
    /** Fill the lists of particles' momenta */
    void getParticleMomentumLists(std::vector<std::string> particleLists);

    /** Calculate the missing momentum in the lab system for this event */
    TVector3 getMissingMomentum();
    /** Calculate the missing momentum in the CMS for this event */
    TVector3 getMissingMomentumCMS();
    /** Calculate the missing energy in the CMS for this event */
    float getMissingEnergyCMS();
    /** Calculate the visible energy in the CMS for this event */
    float getVisibleEnergyCMS();
    /** Calculate the energy for the photons in this event */
    float getTotalPhotonsEnergy();
  };
}

