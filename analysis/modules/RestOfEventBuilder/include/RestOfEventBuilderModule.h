/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /**
   * Creates for each Particle in given ParticleList an RestOfEvent (ROE) dataobject and makes BASF2 relation between them.
   * Takes as first input parameter the name of target particle list, around which the RestOfEvent will be constructed.
   * The ROE particles are taken from default particle lists 'gamma:roe_default', 'pi+:roe_default' and 'K_L0:myroe_default', which defines
   * the default particle type hypotheses: gammas, pions and K-longs.
   * Second input parameter is an array of final-state particle list names, which are preselected by PID, for example:
   * if the module is launched with particleListsInput=['K+:good','e+:good'], it will create ROE object filled with these
   * particle track hypotheses first and all other particles with default particle hypotheses.
   */
  class RestOfEventBuilderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    RestOfEventBuilderModule();

    /** Overridden initialize method */
    virtual void initialize() override;

    /** Overridden event method */
    virtual void event() override;
    /** create usual (host) ROE */
    void createROE();
    /** create nested ROE */
    void createNestedROE();
  private:

    StoreArray<Particle> m_particles; /**< StoreArray of Particles */
    StoreArray<RestOfEvent> m_roeArray; /**< StoreArray of ROEs */
    StoreArray<RestOfEvent> m_nestedROEArray; /**< StoreArray of nested ROEs */
    StoreObjPtr<ParticleList> m_particleList; /**< input particle list */
    StoreArray<ECLCluster> m_eclClusters; /**< StoreArray of ECLCluster */
    StoreArray<KLMCluster> m_klmClusters; /**< StoreArray of KLMCluster */
    StoreArray<Track>      m_tracks; /**< StoreArray of Tracks */

    std::string m_particleListName;  /**< Name of the ParticleList */
    std::string m_nestedMask;  /**< Name of the ParticleList */
    std::vector<std::string> m_particleListsInput;  /**< Name of the input particle lists of pi+ gamma and Klongs*/
    std::string m_nestedROEArrayName; /**< Name of the nested ROE */
    bool m_createNestedROE; /**< Should we create nested ROE? */
    bool m_fromMC; /**< Should we create MC ROE? */
    bool m_useKLMEnergy; /**< Should we use KLM energy in ROE? */
    bool m_builtWithMostLikely; /**< Is the ROE built with most-likely particle lists? */
    /**
    * Adds all particles from input particle lists that are not used in reconstruction of given particle.
    *
    * @param particle particle for which RestOfEvent is determined
    * @param roe pointer to the RestOfEvent to be filled with remaining tracks
    */
    void addRemainingParticles(const Particle* particle, RestOfEvent* roe);

    // cppcheck-suppress unusedPrivateFunction
    /** for debugging purposes */
    void printEvent();

    // cppcheck-suppress unusedPrivateFunction
    /** for debugging purposes */
    void printParticle(const Particle* particle);
  };
}
