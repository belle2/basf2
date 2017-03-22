/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

using namespace std;

namespace Belle2 {
  /// Returns the calibration result from SoftwareTriigerResult for skimming out calibration flagged events
  class TriggerSkimModule : public Module {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    TriggerSkimModule();
    virtual void initialize() override;
    virtual void event() override;

  private:
    vector<string> m_triggerLines;
  };
}
