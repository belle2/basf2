/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsScalar.h>
#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * SVD calibrations from the local runs providing the
   * constants needed to correct the strip time computed with the CoG
   *
   */
  class SVDCoGTimeCalibrations {
  public:
    static std::string name; /**< name of the SVDCoGCalibrationFunction payload */
    typedef SVDCalibrationsBase< SVDCalibrationsScalar< SVDCoGCalibrationFunction > >
    t_payload; /**< typedef for the SVDCoGCalibrationFunction payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDCoGTimeCalibrations() : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDCoGTimeCalibrations: from now on we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });
    }

    /** Return the strip time, given the raw strip time
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number - NOT USED
     * @param raw_time : raw CoG time in ns
     * @param bin : trigger bin (0,1,2,3)
     *
     * Output: double corresponding to the corrected time [ns]
     */
    inline double getCorrectedTime(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const double& raw_time,
      const int& bin
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       strip).calibratedValue(raw_time, bin);

    }

    /** Return the strip time error, given the raw strip time,
     * and tje raw time error
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number - NOT USED
     * @param raw_time : raw CoG time in ns
     * @param raw_timeErr : raw CoG time in ns
     * @param bin : trigger bin (0,1,2,3)
     *
     * Output: double corresponding to the corrected time [ns]
     */
    inline double getCorrectedTimeError(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const double& raw_time,
      const double& raw_timeErr,
      const int& bin
    ) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU),
                              strip).calibratedValueError(raw_time, raw_timeErr, bin);

    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDCoGCalibrationFunction payload */
  };
}
