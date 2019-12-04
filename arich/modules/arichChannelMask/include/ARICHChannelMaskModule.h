/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {
  /// Testing module for collection of calibration data
  class ARICHChannelMaskModule : public CalibrationCollectorModule {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    ARICHChannelMaskModule();

    virtual void prepare() override;
    virtual void collect() override;

  private:

    StoreArray<ARICHDigit> m_ARICHDigits;

  };
}
