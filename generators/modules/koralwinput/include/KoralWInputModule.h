/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;


  protected:

    /** Module parameters */
    std::string m_dataPath;      /**< The path to the KoralW input data files. */
    std::string m_userDataFile;  /**< The filename of the user KoralW input data file. */

    /** Variables */
    KoralW m_generator;        /**< The KoralW generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

  private:

    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

  };

} // end namespace Belle2
