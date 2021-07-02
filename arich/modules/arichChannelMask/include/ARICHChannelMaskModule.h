/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
