/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>

namespace Belle2 {

  /**
   * Extract particles from ROE and fill them in ParticleList
   */
  class ParticleExtractorFromROEModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleExtractorFromROEModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::vector<std::string> m_outputListNames; /**< output ParticleList names */
    std::vector<std::string> m_outputAntiListNames; /**< output anti-ParticleList names */
    std::vector<int> m_pdgCodes; /**< pdg codes of output ParticleList */
    std::vector<int> m_absPdgCodes; /**< pdg codes of output ParticleList */

    std::vector<StoreObjPtr<ParticleList>> m_pLists; /**< output ParticleList names */
    std::vector<StoreObjPtr<ParticleList>> m_antiPLists; /**< output anti-ParticleList names */

    std::string m_maskName; /**< mask name to be applied */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

  };

} // Belle2 namespace

