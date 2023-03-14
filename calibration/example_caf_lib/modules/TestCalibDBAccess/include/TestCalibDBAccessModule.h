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
#include <framework/database/DBArray.h>
#include <calibration/dbobjects/TestCalibObject.h>
#include <calibration/dbobjects/TestCalibMean.h>

namespace Belle2 {
  /// Returns the calibration result from SoftwareTriigerResult for skimming out calibration flagged events
  class TestCalibDBAccessModule : public Module {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    TestCalibDBAccessModule();

    /** Destructor.*/
    virtual ~TestCalibDBAccessModule();

    /// Initialize
    virtual void initialize() override;
    /// Event()
    virtual void event() override;

  private:
    DBObjPtr<TestCalibMean> m_obj_constant; /**< obj_constant */
    DBArray<TestCalibObject> m_array_constants; /**< array_constants */
  };
}
