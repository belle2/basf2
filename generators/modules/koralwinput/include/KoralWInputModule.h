/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KoralW header. */
#include <generators/koralw/KoralW.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <mdst/dataobjects/MCParticleGraph.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * The KoralW Generator module.
   * Generates four fermion final state events using the KoralW FORTRAN generator.
   */
  class KoralWInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    KoralWInputModule();

    /** Destructor. */
    virtual ~KoralWInputModule();

    /** Initializes the module. */
    void initialize() override;

    /** Method is called for each event. */
    void event() override;

    /** Method is called at the end of the event processing. */
    void terminate() override;

  private:

    std::string m_dataPath; /**< The path to the KoralW input data files. */
    std::string m_userDataFile; /**< The filename of the user KoralW input data file. */
    bool m_initialized{false}; /**< True if generator has been initialized. */
    KoralW m_generator; /**< The KoralW generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter database object. */
    InitialParticleGeneration m_initial; /**< InitialParticleGeneration utility. */

  };

} // end namespace Belle2
