/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <calibration/CalibrationCollectorModule.h>
#include <string>

namespace Belle2 {
  /**
   * Calibration Collector Module for PXD hot pixel masking from PXDRawHits
   *
   *
   */
  class PXDRawHotPixelMaskCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDRawHotPixelMaskCollectorModule();
    /** Prepare */
    void prepare() override final;
    /** Collect */
    void collect() override final;

  private:

    /** Required input for  PXDRawHit */
    StoreArray<PXDRawHit> m_pxdRawHit;
    /** Required input for PXD Daq Status */
    StoreObjPtr<PXDDAQStatus> m_storeDaqStatus;

    /** Name of the collection to use for PXDRawHits */
    std::string m_storeRawHitsName;

    /** Minimum charge (ADU) for detecting a hit. */
    int m_0cut;

    /** Utility function to check pixel coordinates */
    inline bool goodHit(const PXDRawHit& rawhit) const
    {
      short u = rawhit.getColumn();
      bool goodU = (u == std::min(std::max(u, short(0)), short(249)));
      short v = rawhit.getRow();
      bool goodV = (v == std::min(std::max(v, short(0)), short(767)));
      return (goodU && goodV);
    }
  };
}
