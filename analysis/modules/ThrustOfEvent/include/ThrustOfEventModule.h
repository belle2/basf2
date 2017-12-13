/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: iorch, Michel Villanueva                                 *                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef THRUSTOFEVENTMODULE_H
#define THRUSTOFEVENTMODULE_H

#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <TVector3.h>

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ThrustOfEvent.h>

#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>
#include <generators/utilities/cm2LabBoost.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ContinuumSuppression/Thrust.h>


namespace Belle2 {
  /**
   * Module to compute Thrust of a particle list, mainly used to compute the thrust of a tau-taubar event.
   *
   *    *
   */
  class ThrustOfEventModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ThrustOfEventModule();

    /** free memory */
    virtual ~ThrustOfEventModule();

    /** Define the physical parameters. llok for them in database. */
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

    /** Count the total number of particles */
    int m_nParticles;

    /**
     * Adds all tracks from StoreArray<Track> that are used in reconstruction of given particle.
     *
     * @param reconstructed particle for which ThrustOfEvent is determined
     * @param pointer to the RestOfEvent to be filled with selected tracks
     * @param selection criteria for the selected tracks
     */
    void addSelectedTracks(const Particle* particle, RestOfEvent* roeECLClusters);

    /**
     * Adds all ECL clusters from StoreArray<ECLCluster> that are used in reconstruction of given particle.
     *
     * @param reconstructed particle for which ThrustOfEvent is determined
     * @param pointer to the RestOfEvent to be filled with selected ECL clusters
     * @param selection criteria for the selected ECL objects
     */
    void addSelectedECLClusters(const Particle* particle, RestOfEvent* roe);

    /**
     * Adds all KLM clusters from StoreArray<KLMCluster> that are used in reconstruction of given particle.
     *
     * @param reconstructed particle for which ThrustOfEvent is determined
     * @param pointer to the RestOfEvent to be filled with selected KLM clusters
     * @param selection criteria for the selected KLM objects
     */
    void addSelectedKLMClusters(const Particle* particle, RestOfEvent* roe);

    TVector3 getThrustOfEvent(std::vector<std::string>  m_particleList);

    /**
     * for debugging purposes
     */
    void printParticle(const Particle* particle);
  };
}

#endif /* THRUSTOFEVENTMODULE_H */
