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

#include <string>

#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/core/Module.h>

namespace Belle2 {
  /// Returns the calibration result from SoftwareTriigerResult for skimming out calibration flagged events
  class TriggerSkimModule : public Module {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    TriggerSkimModule();
    /// Initialize
    virtual void initialize() override;
    /// Event function
    virtual void event() override;

  private:

    StoreObjPtr<SoftwareTriggerResult> m_trigResults; /**< Required input for trigger results */

    std::vector<std::string> m_triggerLines;

    int m_expectedResult;

    std::string m_logicMode;
  };
}
