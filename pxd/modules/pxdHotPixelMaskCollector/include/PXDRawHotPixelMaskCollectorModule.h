/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <calibration/CalibrationCollectorModule.h>
#include <string>

namespace Belle2 {
  /**
   * Calibration Collector Module for PXD hot pixel masking from PXDRawHits
   *
   *    *
   */
  class PXDRawHotPixelMaskCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDRawHotPixelMaskCollectorModule();
    void prepare();
    void collect();

  private:

    /**< Required input for  PXDRawHit */
    StoreArray<PXDRawHit> m_pxdRawHit;

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
