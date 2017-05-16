/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Torben Ferber                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KKGENINPUTMODULE_H
#define KKGENINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/kkmc/KKGenInterface.h>

#include <generators/utilities/InitialParticleGeneration.h>

namespace Belle2 {

  /** The KKGenInput module.
   *  interface for KK2f MC Event Generator
   *  stores generated particles in MCParticles.
   */
  class KKGenInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    KKGenInputModule();

    /** Destructor. */
    virtual ~KKGenInputModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each run. */
    virtual void beginRun();

    /** Method is called for each event. */
    virtual void event();

    /** Method is called at the end of the event processing. */
    virtual void terminate();


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

#endif // KKGENINPUTMODULE_H
