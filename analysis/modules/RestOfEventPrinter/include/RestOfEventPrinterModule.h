/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENTPRINTERMODULE_H
#define RESTOFEVENTPRINTERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <map>

#include <analysis/dataobjects/RestOfEvent.h>

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
    std::string m_whichMask; /**< Print Track mask (track), ECLCluster mask (cluster), or (both)? */
    bool m_fullPrint; /**< True: Print whole masks content. */

    /**
     * Prints out values of the specific ROE  mask
     */
    void printMaskParticles(const std::vector<const Particle*>& maskParticles) const;


  };

} // Belle2 namespace

#endif
