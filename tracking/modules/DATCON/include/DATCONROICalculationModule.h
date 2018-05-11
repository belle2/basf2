/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/DATCONMostProbableHit.h>

#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <pxd/geometry/SensorInfo.h>

#include <root/TVector2.h>
#include <root/TVector3.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>


namespace Belle2 {

  /** The DATCONROICalculationModule calculates ROI from the
    * DATCONMostProbableHit / PXDIntercepts calculated by the
    * DATCONMPHCalculationModule and saves the ROI in a
    * ROIid StoreArray.
    */
  class DATCONROICalculationModule : public Module {
  public:
    /** Constructor.  */
    DATCONROICalculationModule();

    /** Standard Deconstructor */
    virtual ~DATCONROICalculationModule() = default;

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();

  protected:

    /** Members holding module parameters: */

    /** 1. Collections */
    /** Name of the PXDIntercepts array */
    std::string m_storeDATCONPXDInterceptsName;
    /** Name of the PXDIntercepts array */
    std::string m_storeDATCONMPHName;
    /** Name of the DATCONROIids array */
    std::string m_storeDATCONROIidName;

    /** StoreArray for the DATCON-specific "Most Probable Hits" */
    StoreArray<DATCONMostProbableHit> storeDATCONMPHs;
    /** StoreArray of the PXDIntercepts calculated by DATCON */
    StoreArray<PXDIntercept> storeDATCONPXDIntercepts;
    /** StoreArray of the ROIids calculated by DATCON */
    StoreArray<ROIid> storeDATCONROIids;

    /** Continue ROI on neighbouring sensors? (default: true) */
    bool m_ContinueROIonNextSensor;
    /** Fixed-size of ROI size in pixels in u-direction (default: 100) */
    int m_fixedSizeUCells;
    /** Fixed-size of ROI size in pixels in v-direction (default: 150) */
    int m_fixedSizeVCells;

  };//end class declaration
} // end namespace Belle2
