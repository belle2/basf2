/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <string>

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {

  /**
   * prints ROE information and masks to screen
   */
  class RestOfEventPrinterModule : public Module {

  public:

    /**
     * Constructor
     */
    RestOfEventPrinterModule();

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

    std::vector<std::string> m_maskNames; /**< List of all mask names for which the info will be printed */
    bool m_fullPrint; /**< True: Print whole masks content. */
    bool m_unpackComposites; /**< True: Replace composite particles or V0 by their daughters, default is true */
    /**
     * Prints out values of the specific ROE  mask
     */
    void printMaskParticles(const std::vector<const Particle*>& maskParticles) const;


  };

} // Belle2 namespace

