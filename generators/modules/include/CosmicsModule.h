/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/cosmics/SGCosmic.h>

namespace Belle2 {

  /**
   * The Cosmics module
   * Generates tracks with the cosmics generator
   * and store them into the MCParticle class
   */
  class CosmicsModule : public Module {

  public:
    /**
     * Constructor. Sets the module parameters
     */
    CosmicsModule();

    /**
     * Destructor
     */
    virtual ~CosmicsModule() {}

    /**
     * Initializes the module
     */
    void initialize() override;

    /**
     * Method is called for each event
     */
    void event() override;

  protected:

    /**
     * Instance of the cosmics generator
     */
    SGCosmic m_cosmics;
    /**
     * Instance of parameters
     */
    SGCosmic::Parameters m_parameters;
    /**
     * Particle graph to generate MCParticle list
     */
    MCParticleGraph m_particleGraph;

  };

} // end namespace Belle2

