/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <generators/modules/GeneratorBaseModule.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <framework/dataobjects/MCInitialParticles.h>

#include <string>
#include <utility>

namespace Belle2 {

  /** The EvtGenInput module.
   * interface for EvtGen Event Generator
   * stores generated particles in MCParticles.
   */
  class EvtGenInputModule : public GeneratorBaseModule {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    EvtGenInputModule();

    /** Destructor. */
    virtual ~EvtGenInputModule() {}

    /** Initializes the module. */
    virtual void generatorInitialize() override;

    /** Method is called for each run. */
    virtual void beginRun() override;

    /** Method is called for each event. */
    virtual void generatorEvent() override;

  protected:

    /** Create a "beam particle" from LER and HER direction, energy and energy
     * spread and make sure it's inside the given mass window. Also boost it to
     * CMS if needed
     * @param minMass minimum mass for the beam particle to be accepted
     * @param maxMass maximum mass for the beam particle to be accepted
     * @return lorentzvector of the compound beam particle
     */
    MCInitialParticles  createBeamParticle(double minMass = 0.0,
                                           double maxMass = std::numeric_limits<double>::infinity());

    EvtGenInterface m_Ievtgen;  /**< An instance of the EvtGen Interface. */
    std::string m_userDECFileName; /**<  Standard input user decay file. */
    std::string m_DECFileName;     /**<  Standard input decay file.  */
    std::string m_parentParticle;  /**<  Standard input parent particle. */
    int m_inclusiveType;        /**< Inclusive type 0 : generic, 1 : m_inclusiveParticle inclusive, 2 : m_inclusiveParticle + c.c. inclusive */
    bool m_coherentMixing = true;        /**< Decay the B's in coherent or incoherent mode*/
    std::string m_inclusiveParticle; /**< inclusive Particle */
    int m_maxTries; /**< Maximum number of tries for generating the parent particle */
    EvtId m_parentId; /**< EvtGen Id of the parent particle we want to generate */

  private:

    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */
  };

} // end namespace Belle2

