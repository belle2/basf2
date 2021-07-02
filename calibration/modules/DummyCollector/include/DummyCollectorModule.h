/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

namespace Belle2 {
  /// Basically empty Collector, makes only a tiny amount of output data
  class DummyCollectorModule : public CalibrationCollectorModule {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    DummyCollectorModule();

    virtual void prepare() override;
    virtual void collect() override;
  };
}
