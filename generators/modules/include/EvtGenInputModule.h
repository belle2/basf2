/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTGENINPUTMODULE_H
#define EVTGENINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/evtgen/EvtGenInterface.h>

namespace Belle2 {

  /** The EvtGenInput module.
   * interface for EvtGen Event Generator
   * stores generated particles in MCParticles.
   */
  class EvtGenInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    EvtGenInputModule();

    /** Destructor. */
    virtual ~EvtGenInputModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each run. */
    virtual void beginRun();

    /** Method is called for each event. */
    virtual void event();

  protected:

    MCParticleGraph mpg;        /**< An instance of the MCParticle graph. */
    EvtGenInterface m_Ievtgen;  /**< An instance of the EvtGen Interface. */
    std::string m_DECFileName;     /**<  Standard input decay file.  */
    std::string m_userDECFileName; /**<  Standard input user decay file. */
    std::string m_pdlFileName;     /**<  Standard input pdl file.  */
    std::string m_parentParticle;  /**<  Standard input parent particle. */
    int m_inclusiveType;        /**< Inclusive type 0 : generic, 1 : m_inclusiveParticle inclusive, 2 : m_inclusiveParticle + c.c. inclusive */
    std::string m_inclusiveParticle; /**< inclusive Particle */
    bool m_boost2LAB;              /**<  Boosted to LAB frame. */
    double m_EHER;              /**< Energy for HER [GeV]. */
    double m_ELER;              /**< Energy for LER [GeV]. */
    double m_HER_Espread;       /**< Energy spread for HER [GeV]. */
    double m_LER_Espread;       /**< Energy spread for LER [GeV]. */
    double m_crossing_angle;    /**< Beam pipe crossing angle [rad]. */
    double m_angle;             /**< Rotation with respect to e- beampie [rad]. */

  };

} // end namespace Belle2

#endif // EVTGENINPUTMODULE_H
