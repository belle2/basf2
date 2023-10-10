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
        B2DEBUG(20, "SVDClustering: from now on we are using " <<
                this->m_aDBObjPtr -> get_uniqueID()); });

    }


    /** Return the minimum SNR for the seed
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
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

    /** Return the unfolding coefficient for the strip charge
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
     *
     * Output: double corresponding to the unfolding coefficient for the cluster
     */
    inline double getUnfoldingCoeff(
      const Belle2::VxdID& sensorID,
      const bool& isU
    ) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(),
                                       sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(),
                                       m_aDBObjPtr->sideIndex(isU),
                                       0 //strip not relevant
                                      ).UnfoldingCoeff;

    }

    /** Return the minimum SNR for the cluster
     *
     * Input:
     * @param sensorID: identity of the sensor for which the calibration is required
     * @param isU: sensor side, true for p side, false for n side
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
