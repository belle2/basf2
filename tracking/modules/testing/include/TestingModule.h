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
   * simple module to test datastore io
   */
  class TestingModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TestingModule();

    /**  */
    void initialize() override;

    /**  */
    void beginRun() override;

    /**  */
    void event() override;

    /**  */
    void endRun() override;

    /**  */
    void terminate() override;


  private:

  };
}
