/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>

#include <generators/modules/GeneratorBaseModule.h>
#include <generators/kkmc/KKGenInterface.h>
#include <generators/utilities/InitialParticleGeneration.h>

#include <string>

namespace Belle2 {

  /** The KKGenInput module.
   *  interface for KK2f MC Event Generator
   *  stores generated particles in MCParticles.
   */
  class KKGenInputModule : public GeneratorBaseModule {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    KKGenInputModule();

    /** Destructor. */
    virtual ~KKGenInputModule() {}

    /** Initializes the module. */
    virtual void generatorInitialize() override;

    /** Method is called for each run. */
    virtual void beginRun() override;

    /** Method is called for each event. */
    virtual void generatorEvent() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:

    MCParticleGraph mpg; /**< An instance of the MCParticle graph. */
    KKGenInterface m_Ikkgen;  /**< An instance of the KK2f MC Interface. */
    std::string m_KKdefaultFileName; /**<  KKMC default setting file.  */
    std::string m_tauinputFileName; /**<  KKMC setting file for generating fermions.  */
    std::string m_taudecaytableFileName; /**<  tau decay tables in PYTHIA6 manner.  */
    std::string m_KKMCOutputFileName; /**<  KKMC output file previously as set "fort.16".  */

  private:
    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

  };

} // end namespace Belle2
