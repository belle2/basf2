
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CRYINPUTMODULE_H
#define CRYINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/cry/CRY.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <vector>
#include <TFile.h>
#include <TH1D.h>

namespace Belle2 {

  /**
   * The CRY Generator module.
   * Generates cosmic events using the CRY generator.
   */
  class CRYInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    CRYInputModule();

    /** Destructor. */
    virtual ~CRYInputModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:
    /** Module parameters */
    std::string m_setupfile; /**< user setupfile. */
    std::string m_cosmicdatadir; /**< cosmic data (used by CRY for interpolation). */
    double m_acceptLength;  /**< length of a box used to reject quickly non detector particles. */
    double m_acceptWidth;  /**< width of a box used to count particle into acceptance box quickly non detector particles. */
    double m_acceptHeight;  /**< height of a box used to count particle into acceptance box quickly non detector particles. */
    double m_keepLength;  /**< length of a box used to count particle into acceptance box quickly non detector particles. */
    double m_keepWidth;  /**< width of a box used to reject quickly non detector particles. */
    double m_keepHeight;  /**< height of a box used to reject quickly non detector particles. */
    double m_kineticEnergyThreshold;  /**< kinetic energy threshold. */
    double m_timeOffset;  /**< time offset. */
    int m_maxTrials;  /**< maximum number of trials. */

    CRY m_generator;   /**< The CRY generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

  private:

  };


} // end namespace Belle2

#endif /*CRYINPUTMODULE_H */
