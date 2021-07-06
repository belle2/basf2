/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * CollectorTestModules
   *
   */
  class CollectorTestModule : public Module {

  public:

    /** Constructor */
    CollectorTestModule();

    /** Init the module */
    void initialize() override;
    /** Show progress */
    void event() override;
    /** Don't break the terminal */
    void terminate() override;

  protected:

  };
} // end namespace Belle2

