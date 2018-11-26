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
    static std::string name;
    typedef SVDCalibrationsBase< SVDCalibrationsScalar< SVDCoGCalibrationFunction > > t_payload;

    /** Constructor, no input argument is required */
    SVDCoGTimeCalibrations()
      : m_aDBObjPtr(name)
    {}

    /** Return the charge (number of electrons/holes) collected on a specific
     * strip, given the number of ADC counts.
     *
     * Input:
     * @param sensor ID: identity of the sensor for which the
     * calibration is required
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
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU),
                              strip).calibratedValue(raw_time, bin);

    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr;
  };
}

