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
#include <svd/dbobjects/SVDCalibrationsBitmap.h>
#include <framework/database/DBObjPtr.h>


#include <string>

//#include <framework/logging/Logger.h>


namespace Belle2 {

  /** This class defines the wrapper to retrieve the the list
   *  of the hot strips flgged offline.
   *
   */
  class SVDHotStripsCalibrations {
  public:
    static std::string name; /**< name of the SVDHotStripsCalibrations payload */
    typedef SVDCalibrationsBase< SVDCalibrationsBitmap >
    t_payload; /**< typedef of the SVDHotStripsCalibrations payload for all SVD strips*/

    /** Constructor, no input argument is required */
    SVDHotStripsCalibrations(): m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2DEBUG(20, "SVDHotStripsCalibrations: from now on we are using " <<
                this->m_aDBObjPtr -> get_uniqueID()); });
    }


    /** This is the method for getting the offline list of bad strips to be masked.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: boolean value, 1 is the strip has to be masked, 0 otherwise.
     * it throws std::out_of_range if the strip is unknown
     */

    inline float isHot(const VxdID& sensorID, const bool& isU, const unsigned short& strip) const
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
    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDHotStripsCalibrations payload */


  };
}

