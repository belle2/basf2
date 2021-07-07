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

#include <string>

namespace Belle2 {
  /**
   * SVD 6-sample event skim module
   *
   */
  class SVD6SampleEventSkimModule : public Module {
  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVD6SampleEventSkimModule();

    /** Event processor. */
    void event() override;

  };
}
