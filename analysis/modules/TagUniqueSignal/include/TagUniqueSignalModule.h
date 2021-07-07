/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <analysis/VariableManager/Manager.h>
#include <framework/core/Module.h>
#include <string>

namespace Belle2 {
  /**
   * Loops through the particle list finds a unique signal candidate for each event.
   * Usefull for use by the FEI and for strange cases where there are two isSignal
   * flags in the event (for example, clone tracks)
   */
  class TagUniqueSignalModule : public Module {
  public:
    /** Constructor */
    TagUniqueSignalModule();
    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;
    /** Event processor. */
    virtual void event() override;
  private:
    std::string m_particleList;   /**< input ParticleList name */
    std::string m_extraInfoName;  /**< output extra-info name */
    std::string m_targetVariable; /**< Variable which defines signal and background */
    const Variable::Manager::Var* m_targetVar; /**< Pointer to target variable stored in the variable manager */
  };
} // Belle2 namespace
