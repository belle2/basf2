/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/database/DBObjPtr.h>

#include <simulation/dbobjects/ROIParameters.h>


namespace Belle2 {
  /// Module which sets its return value based on the payload whether ROI-finding was enabled for the given run/exp interval or not
  class ROIfindingConditionFromDBModule : public Module {
  public:
    /// Add the module parameters and the description.
    ROIfindingConditionFromDBModule();

    /// Returns true, if ROI-finding is enabled.
    void event() override;

  private:
    /// Internal condition: true if run/exp is in IoV
    bool m_roiEnabled = true;

    /// Configuration parameters for ROIs
    DBObjPtr<ROIParameters> m_roiParameters;
  };

}
