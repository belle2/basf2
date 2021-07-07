/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>

namespace Belle2 {

  /**
   * prints particle list to screen
   */
  class ParticlePrinterModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticlePrinterModule();

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

    StoreObjPtr<ParticleList> m_plist; /**< particle list */

    std::string m_listName;   /**< particle list name */
    bool m_fullPrint;         /**< steering variable */

    std::vector<std::string> m_variables;  /**< names of variables to be printed */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the particle being selected */

    /**
     * Prints out the values of variables specified by the user via the Variables module parameter
     */
    void printVariables(const Particle* particle) const;
  };

} // Belle2 namespace

