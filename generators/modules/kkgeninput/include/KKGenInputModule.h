/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KKGENINPUTMODULE_H
#define KKGENINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/kkmc/KKGenInterface.h>

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

    /** Method is called for each event. */
    virtual void event();

  protected:

    MCParticleGraph mpg;        /**< An instance of the MCParticle graph. */
    KKGenInterface m_Ikkgen;  /**< An instance of the KK2f MC Interface. */
    std::string m_KKdefaultFileName;
    std::string m_tauinputFileName;
    std::string m_taudecaytableFileName;
    std::string m_EvtPDLFileName;
    bool m_boost2LAB;
    double m_EHER;
    double m_ELER;
    double m_HER_Espread;
    double m_LER_Espread;
    double m_crossing_angle;
    double m_angle;

  };

} // end namespace Belle2

#endif // KKGENINPUTMODULE_H
