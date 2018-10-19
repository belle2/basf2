/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsScalar.h>
#include <svd/dbobjects/SVDClusterCuts.h>
#include <svd/dbobjects/SVDHitTimeSelectionFunction.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * calibration of the cluster reconstruction
   *
   */
  class SVDClusterCalibration {
  public:
    static std::string name;
    static std::string time_name;
    typedef SVDCalibrationsBase< SVDCalibrationsScalar<SVDClusterCuts> > t_payload;
    typedef SVDCalibrationsBase< SVDCalibrationsScalar<SVDHitTimeSelectionFunction> > t_time_payload;

    /** Constructor, no input argument is required */
    SVDClusterCalibration()
      : m_aDBObjPtr(name)
      , m_time_aDBObjPtr(time_name)
    {}

    /** Return the corrected cluster position error
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param strip: NOT USED
     * @param isU: sensor side, true for p side, false for n side
     * @param size: cluster size
     * @param raw_error : error provided byt algorithm in the SVDSimpleClusterizer
     *
     * Output: double corresponding to the corrected cluster position error [cm]
     */
    inline double getCorrectedClusterPositionError(
      const Belle2::VxdID& sensorID,
      const bool& isU, const int& size,
      const double& raw_error
    ) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU)).getCorrectedValue(raw_error, size);

    }

    /** Return the minimum SNR for the seed
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param strip: NOT USED
     * @param isU: sensor side, true for p side, false for n side
     *
     * Output: double corresponding to the minimum SNR for the seed
     */
    inline double getMinSeedSNR(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU)).minSeedSNR;

    }

    /** Return the minimum SNR for the adjacent
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param strip: NOT USED
     * @param isU: sensor side, true for p side, false for n side
     *
     * Output: double corresponding to the minimum SNR for the adjacent
     */
    inline double getMinAdjSNR(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU)).minAdjSNR;

    }

    /** Return the minimum SNR for the cluster
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param strip: NOT USED
     * @param isU: sensor side, true for p side, false for n side
     *
     * Output: double corresponding to the minimum SNR for the cluster
     */
    inline double getMinClusterSNR(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU)).minClusterSNR;

    }


    /** Return whther the cluster is estimated to be in time with the event or off-time
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param strip: NOT USED
     * @param isU: sensor side, true for p side, false for n side
     *
     * Output: double corresponding to the minimum SNR for the cluster
     */
    inline bool isClusterInTime(
      const Belle2::VxdID& sensorID,
      const bool& isU,
      const double& svdTime, const double& svdTimeError = 0,
      const double& t0 = 0, const double& t0Error = 0
    ) const
    {
      return m_time_aDBObjPtr->get(sensorID.getLayerNumber(),
                                   sensorID.getLadderNumber(),
                                   sensorID.getSensorNumber(),
                                   sensorID.getSensorNumber(),
                                   m_aDBObjPtr->sideIndex(isU)).isOnTime(svdTime, svdTimeError, t0, t0Error);

    }


    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr;
    DBObjPtr< t_time_payload > m_time_aDBObjPtr;
  };
}

