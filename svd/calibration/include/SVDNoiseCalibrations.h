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
#include <string>

//#include <framework/logging/Logger.h>


namespace Belle2 {

  /** This class defines the dbobject and the method to access SVD
   * calibrations from the noise local runs. It provides the strip noise
   * in ADC units.
   *
   * Currently the returned values are the default ones and they
   * are not read from the DB.
   *
   */
  class SVDNoiseCalibrations {
  public:
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector<float> > t_payload;

    /** Constructor, no input argument is required */
    SVDNoiseCalibrations(): m_aDBObjPtr(name)
    {}


    /** This is the method for getting the noise.
     * Input:
     * @param sensor ID: identitiy of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: integer corresponding to the strip noise in ADC counts.
     * it throws std::out_of_range if the strip is unknown
     */
    float getNoise(VxdID sensorID, bool isU , unsigned short strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                              strip);
    }

    void setNoise(VxdID sensorID, bool isU , unsigned short strip, float stripNoise)
    {
      m_aDBObjPtr->set(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                       strip, stripNoise);
    }

  private:
    DBObjPtr< t_payload > m_aDBObjPtr;


  };
}

