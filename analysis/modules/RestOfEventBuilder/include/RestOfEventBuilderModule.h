/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENTBUILDERMODULE_H
#define RESTOFEVENTBUILDERMODULE_H

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

namespace Belle2 {

  /**
   * Creates for each Particle in given ParticleList an RestOfEvent dataobject and makes BASF2 relation between them.
   *
   *    *
   */
  class RestOfEventBuilderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    RestOfEventBuilderModule();

    /**  */
    virtual void initialize() override;

    /** n */
    virtual void event() override;


  private:

    std::string m_particleList;  /**< Name of the ParticleList */

    /**
     * Adds all tracks from StoreArray<Track> that are not used in reconstruction of given particle.
     *
     * @param reconstructed particle for which RestOfEvent is determined
     * @param pointer to the RestOfEvent to be filled with remaining tracks
     * @param selection criteria for the remaining tracks
     */
    void addRemainingTracks(const Particle* particle, RestOfEvent* roe);

    /**
     * Adds all ECL clusters from StoreArray<ECLCluster> that are not used in reconstruction of given particle.
     *
     * @param reconstructed particle for which RestOfEvent is determined
     * @param pointer to the RestOfEvent to be filled with remaining ECL clusters
     * @param selection criteria for the remaining ECL objects
     */
    void addRemainingECLClusters(const Particle* particle, RestOfEvent* roe);

    /**
     * Adds all KLM clusters from StoreArray<KLMCluster> that are not used in reconstruction of given particle.
     *
     * @param reconstructed particle for which RestOfEvent is determined
     * @param pointer to the RestOfEvent to be filled with remaining KLM clusters
     * @param selection criteria for the remaining KLM objects
     */
    void addRemainingKLMClusters(const Particle* particle, RestOfEvent* roe);

    /**
     * for debugging purposes
     */
    void printEvent();

    /**
     * for debugging purposes
     */
    void printParticle(const Particle* particle);
  };
}

#endif /* RESTOFEVENTBUILDERMODULE_H */
