/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsScalar.h>
#include <svd/dbobjects/SVDHitTimeSelectionFunction.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * calibration of the cluster reconstruction
   *
   */
  class SVDHitTimeSelection {
  public:
    static std::string name; /**< name of SVDHitTimeSelectionFunction payload */
    typedef SVDCalibrationsBase< SVDCalibrationsScalar<SVDHitTimeSelectionFunction> >
    t_payload; /**< typedef for the of SVDHitTimeSelectionFunction  payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDHitTimeSelection()
      : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDHitTimeSelection: from now on we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });
    }



    /** Return whether the cluster is estimated to be in time with the event or off-time
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     * @param svdTime: cluster time
     * @param svdTimeError: cluster time error
     * @param t0: event t0
     * @param t0Error: event t0 error
     *
     * Output: bool true if the svd hit time is compatible with t0
     */
    inline bool isClusterInTime(
      const Belle2::VxdID& sensorID,
      const bool& isU,
      const double& svdTime, const double& svdTimeError = 0,
      const double& t0 = 0, const double& t0Error = 0
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0 //strip not relevant
                                      ).isInTime(svdTime, svdTimeError, t0, t0Error);

    }

    /** Return whether the cluster is estimated to be in time with the event or off-time
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param uTime: U-Side cluster time
     * @param vTime: V-Side cluster time
     *
     * Output: bool true if the U and V cluster times are compatible
     */
    inline bool areClusterTimesCompatible(
      const Belle2::VxdID& sensorID,
      const double& uTime, const double& vTime = 0
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(true), // side not relevant
                                       0 // strip not relevant
                                      ).areClustersInTime(uTime, vTime);
    }


    /** Return the version of the function used to determine whether the
     * cluster time is acceptable at the SP creation
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     *
     * Output:
     */
    inline int getTimeSelectionFunction(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0 // strip not relevant
                                      ).getFunctionID();

    }

    /** Return the min value of the cluster time to use it for reconstruction.
     * this function is used in the calibration monitoring
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     *
     * Output:
     */
    inline float getMinClusterTime(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0 // strip not relevant
                                      ).getMinTime();

    }


    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDHitTimeSelectionFunction paylaod */
  };
}
