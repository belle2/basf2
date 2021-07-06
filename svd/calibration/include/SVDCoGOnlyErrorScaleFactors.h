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
#include <svd/dbobjects/SVDPosErrScaleFactors.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * scaling factors for the CoGOnly position algorithm
   *
   */
  class SVDCoGOnlyErrorScaleFactors {
  public:
    static std::string name; /**< name of SVDPosErrScaleFactors  payload*/
    typedef SVDCalibrationsBase< SVDCalibrationsScalar<SVDPosErrScaleFactors> >
    t_payload;  /**< typedef for the of SVDPosErrScaleFactors  payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDCoGOnlyErrorScaleFactors()
      : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2DEBUG(20, "SVDCoGOnlyErrorScaleFactors: from now on we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });
    }
    /** Return the corrected cluster position error
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     * @param size: cluster size
     * @param raw_error : error provided by the algorithm in the SVDClusterizer
     *
     * Output: double corresponding to the corrected cluster position error [cm]
     */
    inline double getCorrectedClusterPositionError(
      const Belle2::VxdID& sensorID,
      const bool& isU, const int& size,
      const double& raw_error
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0).getCorrectedValue(raw_error, size);

    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDPosErrScaleFactors  payload */
  };
}
