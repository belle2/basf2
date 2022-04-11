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
#include <svd/dbobjects/SVDChargeSimCal.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access SVD
  * simulation calibrations; coupling constants and Geant4 electron weight.
  */
  class SVDChargeSimulationCalibrations {
  public:
    static std::string name; /**< name of the SVDChargeSimulationCalibrations payload */
    typedef SVDCalibrationsBase< SVDCalibrationsScalar< SVDChargeSimCal > >
    t_payload; /**< typedef of the SVDChargeSimulationCalibrations payload*/

    /** Constructor, no input argument is required */
    SVDChargeSimulationCalibrations(): m_aDBObjPtr(name)
    {
      m_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2DEBUG(20, "SVDChargeSimulationCalibrations: from now on we are using " <<
                this->m_aDBObjPtr -> get_uniqueID()); });

    }

    /** Return coupling constant.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     * @param couplingName: coupling name, specify which coupling to return. Possible values are:
     * "C0"          |  Coupling from readout implant to its readout channel
     * "C1"          |  Coupling from first neighbour implant (floating) to readout channel
     * "C2"          |  Coupling from second neighbour implant (readout) to readout channel
     * "C3"          |  Coupling from third neighbour implant (floating) to readout channel
     * "APVCoupling" | Coupling between two adjacent channels done at APV level after charge sharing
     * Output: float corresponding to a given coupling constant.
     */
    inline float getCouplingConstant(const VxdID& sensorID, const bool& isU, const std::string& couplingName) const
    {
      float coupling_constant = 0;
      try {
        coupling_constant = m_aDBObjPtr->getReference(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                                                      sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU), 0).couplingConstant.at(couplingName);
      } catch (const std::out_of_range& oor) {
        B2ERROR("Cannot find coupling constant " << couplingName << " in database.");
      }
      return coupling_constant;
    }

    /** Return Geant4 electron weight.
     * Input:
     * @param sensorID: identitiy of the sensor for which the calibration is required
     * @param isU: sensor side, true for p (u) side, false for n (v) side
     *
     * Output: float corresponding to a Geant4-electron to real-electron weight.
     */
    inline float getElectronWeight(const VxdID& sensorID, const bool& isU) const
    {
      return m_aDBObjPtr->getReference(sensorID.getLayerNumber(), sensorID.getLadderNumber(),
                                       sensorID.getSensorNumber(), m_aDBObjPtr->sideIndex(isU), 0).electronWeight;
    }

    /** returns the unique ID of the payload */
    TString getUniqueID() { return m_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid() { return m_aDBObjPtr.isValid(); }

  private:
    DBObjPtr <t_payload> m_aDBObjPtr; /**< the SVDChargeSimulationCalibrations payload */

  };

}
