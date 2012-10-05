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

#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/MCParticleGraph.h>
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

    /** Method is called for each event. */
    virtual void event();

  protected:

    MCParticleGraph mpg;        /**< An instance of the MCParticle graph. */
    EvtGenInterface m_Ievtgen;  /**< An instance of the EvtGen Interface. */
    std::string m_DECFileName;
    std::string m_userDECFileName;
    std::string m_pdlFileName;
    std::string m_parentParticle;
    bool m_boost2LAB;
    double m_EHER;
    double m_ELER;
    double m_HER_Espread;
    double m_LER_Espread;
    double m_crossing_angle;
    double m_angle;

  };

} // end namespace Belle2

#endif // EVTGENINPUTMODULE_H
