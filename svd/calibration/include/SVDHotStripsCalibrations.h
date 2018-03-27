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
#include <svd/dbobjects/SVDCalibrationsBitmap.h>
#include <framework/database/DBObjPtr.h>


#include <string>

//#include <framework/logging/Logger.h>


namespace Belle2 {

  /** This class defines the dbobject and the method to access SVD
   * calibrations from the noise local runs. It provides the strip noise
   * in ADC units.
   *
   *
   */
  class SVDHotStripsCalibrations {
  public:
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsBitmap > t_payload;

    /** Constructor, no input argument is required */
    SVDHotStripsCalibrations(): m_aDBObjPtr(name)
    {}


    /** This is the method for getting the offline list of bad strips to be masked.
     * Input:
     * @param sensor ID: identitiy of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: boolean value, 1 is the strip has to be masked, 0 otherwise.
     * it throws std::out_of_range if the strip is unknown
     */

    inline float isHot(const VxdID& sensorID, const bool& isU , const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                              strip);
    }

    //** temporary function for debugging of the module */
    inline void set(const VxdID& sensorID, const bool& isU , const unsigned short& strip, bool isHot)
    {
      m_aDBObjPtr->set(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                       strip, isHot);
    }


  private:
    DBObjPtr< t_payload > m_aDBObjPtr;


  };
}

