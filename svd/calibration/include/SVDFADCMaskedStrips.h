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

  /** This class defines the dbobject and the method to access strips which
   * are masked at FADC level.
   *
   */
  class SVDFADCMaskedStrips {
  public:
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsBitmap > t_payload;

    /** Constructor, no input argument is required */
    SVDFADCMaskedStrips(): m_aDBObjPtr(name)
    {}


    /** This is the method for getting the comprehensive list of masked strips
     * at FADC level. The list includes both strips masked accordingly to
     * the calibration results and strips permanently masked regardless the
     * outcome of the calibration runs.
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: boolean value, 1 is masked, 0 otherwise.
     * it throws std::out_of_range if the strip is unknown
     */

    inline float isMasked(const VxdID& sensorID, const bool& isU , const unsigned short& strip) const
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
    DBObjPtr< t_payload > m_aDBObjPtr;


  };
}

