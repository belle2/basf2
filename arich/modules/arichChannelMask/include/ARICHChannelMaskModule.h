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

  /**
   * ARICH channel-mask calibration: data collector.
   */
  class ARICHChannelMaskModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    ARICHChannelMaskModule();

    /**
     * Preparation of data collection.
     */
    virtual void prepare() override;

    /**
     * Collection (called for each event).
     */
    virtual void collect() override;

  private:

    /** Digits. */
    StoreArray<ARICHDigit> m_ARICHDigits;

  };
}
