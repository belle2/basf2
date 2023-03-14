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
#include <framework/database/DBObjPtr.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * SVD time fudge factors for MC reconstruction
   */
  class SVDMCClusterTimeFudgeFactor {
  public:
    static std::string name; /**< name of the SVDMCClusterTimeFudgeFactor payload */
    typedef SVDCalibrationsBase<SVDCalibrationsScalar< float >>
                                                             t_payload; /**< typedef for the SVDMCClusterTimeFudgeFactor payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDMCClusterTimeFudgeFactor() : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([this](const std::string&) -> void
      { B2DEBUG(20, "SVDMCClusterTimeFudgeFactor: from now on we are using " << this->m_aDBObjPtr->get_uniqueID()); });
    }

    /** Return the MC fudge factor
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU sensor side, true for p side, false for n side
     *
     * Output: double corresponding to the fudge factor
     */
    inline double getFudgeFactor(
      const Belle2::VxdID& sensorID,
      const bool& isU) const
    {
      return m_aDBObjPtr->get(sensorID.getLayerNumber(),
                              sensorID.getLadderNumber(),
                              sensorID.getSensorNumber(),
                              m_aDBObjPtr->sideIndex(isU),
                              1);
    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }

  private:
    DBObjPtr<t_payload> m_aDBObjPtr; /**< SVDMCClusterTimeFudgeFactor payload */
  };
}
