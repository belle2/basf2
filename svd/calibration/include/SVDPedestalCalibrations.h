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
   * calibrations from the noise local runs. It provides the strip pedestal
   * in ADC units.
   *
   *
   */
  class SVDPedestalCalibrations {
  public:
    static std::string name; /**< name of the SVDPedestalCalibrations payload*/
    typedef SVDCalibrationsBase< SVDCalibrationsVector< float > >
    t_payload; /**< typedef of the SVDPedestalCalibrations payload of all SVD strips*/

    /** Constructor, no input argument is required */
    SVDPedestalCalibrations(): m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2DEBUG(20, "SVDPedestalCalibrations: from now on we are using " <<
                this->m_aDBObjPtr -> get_uniqueID()); });
    }


    /** This is the method for getting the pedestal.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: float corresponding to the strip pedestal in ADC counts.
     * it throws std::out_of_range if the strip is unknown
     */
    inline float getPedestal(const VxdID& sensorID, const bool& isU, const unsigned short& strip) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                              strip);
    }

    /*    inline void setPedestal(const VxdID& sensorID, const bool& isU , const unsigned short& strip, float stripPedestal)
    {
      m_aDBObjPtr->set(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU),
                       strip, stripPedestal);
    }
    */

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }

  private:
    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDPedestalCalibrations payload*/


  };
}

