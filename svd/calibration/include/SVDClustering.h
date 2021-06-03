/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsScalar.h>
#include <svd/dbobjects/SVDClusterCuts.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * calibration of the cluster reconstruction
   *
   */
  class SVDClustering {
  public:
    static std::string name; /**< name of SVDClusterCuts  payload*/
    typedef SVDCalibrationsBase< SVDCalibrationsScalar<SVDClusterCuts> >
    t_payload;  /**< typedef for the of SVDClusterCuts  payload of all SVD sensors*/

    /** Constructor, no input argument is required */
    SVDClustering()
      : m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDClustering: from now on we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });

    }


    /** Return the minimum SNR for the seed
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     *
     * Output: double corresponding to the minimum SNR for the seed
     */
    inline double getMinSeedSNR(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0).minSeedSNR;

    }

    /** Return the minimum SNR for the adjacent
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     *
     * Output: double corresponding to the minimum SNR for the adjacent
     */
    inline double getMinAdjSNR(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0 //strip not relevant
                                      ).minAdjSNR;

    }

    /** Return the minimum SNR for the cluster
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: NOT USED
     *
     * Output: double corresponding to the minimum SNR for the cluster
     */
    inline double getMinClusterSNR(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0 // strip not relevant
                                      ).minClusterSNR;

    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }


  private:

    DBObjPtr< t_payload > m_aDBObjPtr; /**< SVDClusterCuts  payload */
  };
}
