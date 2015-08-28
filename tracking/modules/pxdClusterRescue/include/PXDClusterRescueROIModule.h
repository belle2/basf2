/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERRESCUEROIMODULE_H
#define PXDCLUSTERRESCUEROIMODULE_H

#include <framework/core/Module.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/ROIid.h>
#include <TVector3.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <genfit/MeasuredStateOnPlane.h>

namespace Belle2 {

  /**
   * Extrapolate tracks to PXD and define region of interests (RIO).
   * Then write ROI information as ROIids to data store.
   */

  class PXDClusterRescueROIModule : public Module {
  public:
    PXDClusterRescueROIModule();
    virtual ~PXDClusterRescueROIModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void terminate();

    /**
     * worker function
     */
    virtual void event();

    /**
     * check whether sensor is in range of track extrapolation
     */
    virtual bool isSensorInRange(TVector3 gfTrackPosition, VXD::SensorInfoBase& pxdSensorInfo);

    /**
     * extract ROI (ROIid object) from track extrapolation
     */
    virtual bool getROI(ROIid* ROI, genfit::MeasuredStateOnPlane gfTrackState, VXD::SensorInfoBase& pxdSensorInfo);

  private:

  protected:
    std::string m_gfTracksColName; /**< Name of genfit tracks in DataStore */
    std::string m_ROIidColName; /**< Name of ROIids in DataStore */
    const float m_pxdLayerRadius[2] = {1.42854, 2.21218}; /**< mean PXD layer radius for both layers */
    float m_factorSigmaUV; /** Factor on uncertainties of U and V position after extrapolation */
    float m_tolerancePhi; /** Tolerance by finding sensor in phi coordinate (radians) */
    float m_toleranceZ; /** Tolerance by finding sensor in z coordinate (cm) */
    int m_maxROILengthU; /** Maximum ROI length in U (pixels) */
    int m_maxROILengthV; /** Maximum ROI length in V (pixels) */
  };

}

#endif
