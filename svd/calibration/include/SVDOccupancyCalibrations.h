/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: Do not try to fry it with water. Use only olive oil.          *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsVector.h>
#include <framework/database/DBObjPtr.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>

#include <string>

//#include <framework/logging/Logger.h>


namespace Belle2 {

  /** This class defines the dbobject and the method to access SVD
   * calibrations from the noise local runs. It provides the strip noise
   * in ADC units.
   *
   *
   */
  class SVDOccupancyCalibrations {
  public:
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_payload;

    static std::string threshold_name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_threshold_payload;

    /** Constructor, no input argument is required */
    SVDOccupancyCalibrations()
      : m_aDBObjPtr(name)
      , m_threshold_aDBObjPtr(threshold_name)
    {}


    /** This is the method for getting the occupancy, or the deviation from the average, still to be decided.
     * Input:
     * @param sensor ID: identitiy of the sensor for which the
     * calibration is required
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


    /** This method provides the threshold on the occupancy deviation per strip which is set to apply the offline masking.
    * Input:
    * @param sensor ID: identitiy of the sensor for which the
    * calibration is required
    * @param isU: sensor side, true for p (u) side, false for n (v) side
    * @param strip: strip number
    *
    * Output: float corresponding to the set threshold applied fo r masking.
    */
    inline float getOccupancyThreshold(const VxdID& sensorID, const bool& isU , const unsigned short& strip) const
    {
      return m_threshold_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                                        sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                                        strip);
    }

    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

  private:
    DBObjPtr< t_payload > m_aDBObjPtr;
    DBObjPtr< t_threshold_payload > m_threshold_aDBObjPtr;


  };
}

