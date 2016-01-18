/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>
#include <fstream>

namespace Belle2 {


  /**
   * This module writes out information about the current ROE object in the datastore
   * which can than later be used in a deep neural network to train the deepFEI algorithm.
   */

  class DeepFEIExtractorModule : public Module {
  public:

    /**
     * Constructor
     */
    DeepFEIExtractorModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Terminates the module.
     */
    virtual void terminate() override;

  private:
    std::string m_filename; /** name of output file */
    std::ofstream m_file; /** output file */

  };

} // Belle2 namespace

