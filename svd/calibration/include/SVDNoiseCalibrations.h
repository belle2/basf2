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
  class SVDNoiseCalibrations {
  public:
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > > t_payload;

    /** Constructor, no input argument is required */
    SVDNoiseCalibrations(): m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDNoiseCalibrations: from now one we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });
    }


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
    inline float getNoise(const VxdID& sensorID, const bool& isU , const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                              strip);
    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


    /** This method provides the correct noise conversion into
    * electrons, taking into account that the noise is the result
    * of an ADC output average, so it is not an integer, but
    * a float number.
    * Input:
    * @param sensor ID: identitiy of the sensor for which the
    * calibration is required
    * @param isU: sensor side, true for p (u) side, false for n (v) side
    * @param strip: strip number
    *
    * Output: float corresponding to the strip noise in electrons.
    */
    inline float getNoiseInElectrons(const VxdID& sensorID, const bool& isU , const unsigned short& strip) const
    {
      SVDPulseShapeCalibrations m_pulseShape;
      return m_pulseShape.getChargeFromADC(sensorID, isU, strip, getNoise(sensorID, isU, strip));
    }

    /*    inline void setNoise(const VxdID& sensorID, const bool& isU , const unsigned short& strip, float stripNoise)
    {
      m_aDBObjPtr->set(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                       strip, stripNoise);
    }
    */

  private:
    DBObjPtr< t_payload > m_aDBObjPtr;


  };
}

