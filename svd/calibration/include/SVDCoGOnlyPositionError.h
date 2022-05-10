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
#include <svd/dbobjects/SVDPositionErrorFunction.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * SVD position error parameters and provide the position error
   * for the CoGOnly algorithm
   */
  class SVDCoGOnlyPositionError {
  public:
    static std::string name; /**< name of the SVDPositionErrorFunction payload */
    typedef SVDCalibrationsBase< SVDCalibrationsScalar< SVDPositionErrorFunction > >
    t_payload; /**< typedef for the SVDPositionErrorFunction payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDCoGOnlyPositionError() : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2DEBUG(20, "SVDCoGOnlyPositionError: from now on we are using " <<
                this->m_aDBObjPtr -> get_uniqueID()); });
    }

    /** Return the position error
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU sensor side, true for p side, false for n side
     * @param strip strip number - NOT USED
     * @param clSNR cluster SNR
     * @param clSize cluster size
     * @param pitch sensor pitch
     *
     * Output: double corresponding to the position error
     */
    inline double getPositionError(
      const Belle2::VxdID& sensorID,
      const bool& isU, const unsigned short& strip,
      const double& clSNR,
      const int& clSize,
      const float& pitch
    ) const
    {
      return pitch * m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                               sensorID.getLadderNumber(),
                                               sensorID.getSensorNumber(),
                                               m_aDBObjPtr->sideIndex(isU),
                                               strip).getPositionError(clSNR, clSize);

    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDPositionErrorFunction payload */
  };
}
