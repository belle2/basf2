/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KORALWINPUTMODULE_H
#define KORALWINPUTMODULE_H

#include <framework/core/Module.h>
#include <generators/koralw/KoralW.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <generators/utilities/InitialParticleGeneration.h>

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
    int m_boostMode;             /**< The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle). */
    std::string m_dataPath;      /**< The path to the KoralW input data files. */
    std::string m_userDataFile;  /**< The filename of the user KoralW input data file. */
    int m_seed;                  /**< The random seed of the generator. */

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


#endif /* KORALWINPUTMODULE_H */
