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
#include <svd/dbobjects/SVDCalibrationsVector.h>
#include <framework/database/DBObjPtr.h>

#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the method to access SVD
   * calibrations from the noise local runs. It provides the strip noise
   * in ADC units.
   *
   *
   */
  class SVDOccupancyCalibrations {
  public:
    static std::string name; /**< name of the ccupancy payload */
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_payload; /**< typedef of the Occupancy payload of all SVD strips*/

    // unused at the moment, but do not forget it!
    //    static std::string threshold_name; /**< name of the threshold payload */
    //    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_threshold_payload; /**< typedef of the threshold payload  of all SVD strips*/

    /** Constructor, no input argument is required */
    SVDOccupancyCalibrations()
      : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDOccupancyCalibrations: from now on we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });
    }


    /** This is the method for getting the occupancy, or the deviation from the average, still to be decided.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: float corresponding to the strip occupancy.
     * it throws std::out_of_range if the strip is unknown
     */
    inline float getOccupancy(const VxdID& sensorID, const bool& isU , const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                              strip);
    }


    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }

  private:
    DBObjPtr< t_payload > m_aDBObjPtr; /**< payload of the occupancy*/


  };
}

