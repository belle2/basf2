/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/dbobjects/SVDCalibrationsBase.h>
#include <svd/dbobjects/SVDCalibrationsVector.h>
#include <svd/dbobjects/SVDSimCal.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access SVD
  * simulation calibrations; coupling constants and Geant4 electron weight.
  */
  class SVDSimulationCalibrations {
  public:
    static std::string name; /**< name of the SVDSimulationCalibrations payload */
    typedef SVDCalibrationsBase< SVDCalibrationsVector<SVDSimCal>>
                                                                t_payload; /**< typedef of the SVDSimulationCalibrations payload*/

    /** Constructor, no input argument is required */
    SVDSimulationCalibrations(): m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDSimulationCalibrations: from now on we are using " <<
        this->m_aDBObjPtr -> get_uniqueID()); });

    }

    /** Return coupling constant.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     * @param couplingName: coupling name, specify which coupling (C0, C1, ...) to return
     *
     * Output: float corresponding to a given coupling constant.
     */
    inline float getCouplingConstant(const VxdID& sensorID, const bool& isU, const unsigned short& strip,
                                     const std::string& couplingName) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU), strip).couplingConstant.at(couplingName);
    }

    /** Return Geant4 electron weight.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param strip: strip number
     *
     * Output: float corresponding to a Geant4-electron to real-electron weight.
     */
    inline float getElectronWeight(const VxdID& sensorID, const bool& isU, const unsigned short& strip) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU), strip).electronWeight;
    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }

  private:
    DBObjPtr <t_payload> m_aDBObjPtr; /**< the SVDSimulationCalibrations payload */

  };

}