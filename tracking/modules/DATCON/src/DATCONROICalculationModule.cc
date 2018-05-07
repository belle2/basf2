/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONROICalculationModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DATCONROICalculation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DATCONROICalculationModule::DATCONROICalculationModule() : Module()
{
  //Set module properties
  setDescription("DATCONROICalculationModule: Calculates ROI from the "
                 "extrapolated hits (MPH), one ROI for each MPH.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("DATCONPXDIntercepts", m_storeDATCONPXDInterceptsName,
           "Name of the DATCONPXDIntercepts StoreArray", string(""));
  addParam("DATCONMPHs", m_storeDATCONMPHName,
           "Name of the DATCONMPH StoreArray", string(""));
  addParam("DATCONROIids", m_storeDATCONROIidName,
           "Name of the ROIid StoreArray for ROI created by DATCON", string(""));

  addParam("continueROIonNextSensor", m_ContinueROIonNextSensor,
           "Continue the ROI on the next sensor when they are close to the edge?", bool(true));
  addParam("useFixeSizeROI", m_useFixedSize,
           "Use ROI of fixed size for u and v?", bool(true));
  addParam("fixedSizeUCells", m_fixedSizeUCells,
           "Fixed size of the ROI in u-direction.", int(100));
  addParam("fixedSizeVCells", m_fixedSizeVCells,
           "Fixed size of the ROI in v-direction.", int(100));

  addParam("multiplicativeU", m_multiplicativeU,
           "Factor for u-direction ROI size in case of variable ROI size.", int(1000));
  addParam("additiveU", m_additiveU,
           "Additive term for u-direction ROI size in case of variable ROI size.", int(100));
  addParam("multiplicativeV", m_multiplicativeV,
           "Factor for v-direction ROI size in case of variable ROI size.", int(1000));
  addParam("additiveV", m_additiveV,
           "Additive term for v-direction ROI size in case of variable ROI size.", int(100));


}


void DATCONROICalculationModule::initialize()
{

  m_storeDATCONPXDIntercepts.isRequired(m_storeDATCONPXDInterceptsName);
  m_storeDATCONPXDInterceptsName = m_storeDATCONPXDIntercepts.getName();

  m_storeDATCONMPHs.isRequired(m_storeDATCONMPHName);
  m_storeDATCONMPHName = m_storeDATCONMPHs.getName();

  m_storeDATCONROIids.registerInDataStore(m_storeDATCONROIidName);
  m_storeDATCONROIidName = m_storeDATCONROIids.getName();

}

/*
* Calculate Region of Interests (ROI).
*/
void
DATCONROICalculationModule::event()
{
  int uSize = m_fixedSizeUCells;
  int vSize = m_fixedSizeVCells;
  /** Reminder: 250 px in u-direction = r-phi, in total 768 (512+256) px in v-direction = z */
  double uCoordinate, vCoordinate;
  int uCell, vCell;
  int uCellDownLeft, vCellDownLeft, uCellUpRight, vCellUpRight;
  int uCellDownLeft_tmp, vCellDownLeft_tmp, uCellUpRight_tmp, vCellUpRight_tmp;
  int sensorChange = 0, ladderChange = 0;
  int uCellsRest = 0, vCellsRest = 0;
  int uCells = 250, vCells = 768;
  VxdID MPHSensorID, nextSensorID;
  TVector2 localPosition;

  double qualityOfHit;
  int sensorNumber;
  int layerNumber;
  int ladderNumber;
  const PXD::SensorInfo* currentSensor;

  ROIid DATCONROIid;

  for (auto& datconmph : m_storeDATCONMPHs) {
    MPHSensorID = datconmph.getSensorID();
    localPosition = datconmph.getLocalCoordinate();
    qualityOfHit = datconmph.getQualityOfHit();

    uCoordinate = localPosition.X();
    vCoordinate = localPosition.Y();

    currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(MPHSensorID));

    uCell = currentSensor->getUCellID(uCoordinate, vCoordinate, false);
    vCell = currentSensor->getVCellID(vCoordinate, false);
    uCells = currentSensor->getUCells();
    vCells = currentSensor->getVCells();

    sensorNumber = MPHSensorID.getSensorNumber();
    layerNumber  = MPHSensorID.getLayerNumber();
    ladderNumber = MPHSensorID.getLadderNumber();

    /* Determine size of ROI */
    if (m_useFixedSize) {
      uSize = m_fixedSizeUCells;
      vSize = m_fixedSizeVCells;
    } else {
      // This is only for trying the 'multiplicative' ansatz
      uSize = (int)(4500 * qualityOfHit + 80);
      vSize = (int)(4500 * qualityOfHit + 50);
    }

    /** Lower left corner */
    uCellDownLeft = uCell - uSize / 2;
    vCellDownLeft = vCell - vSize / 2;

    if (sensorNumber == 1) {
      if (vCellDownLeft < 0) {
        vCellDownLeft = 0;
      }
    } else if (sensorNumber == 2) {
      if (vCellDownLeft < 0) {
        vCellDownLeft = 0;
        sensorChange = -1;
        vCellsRest = vSize / 2 - (vCell - vCellDownLeft);
      }
    }

    if (uCellDownLeft < 0) {
      uCellDownLeft = 0;
      ladderChange = -1;
      uCellsRest = uSize / 2 - (uCell - uCellDownLeft);
    }

    /** Upper right corner */
    uCellUpRight = uCell + uSize / 2;
    vCellUpRight = vCell + vSize / 2;

    if (sensorNumber == 1) {
      if (vCellUpRight > vCells - 1) {
        vCellUpRight = vCells - 1;
        sensorChange = +1;
        vCellsRest = vSize / 2 - (vCellUpRight - vCell);
      }
    } else if (sensorNumber == 2) {
      if (vCellUpRight >= vCells) {
        vCellUpRight = vCells - 1;
      }
    }

    if (uCellUpRight >= uCells) {
      uCellUpRight = uCells - 1;
      ladderChange = +1;
      uCellsRest = uSize / 2 - (uCellUpRight - uCell);
    }

    m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft, uCellUpRight, vCellDownLeft, vCellUpRight, MPHSensorID));

    if (m_ContinueROIonNextSensor) {

      /** ROI covering several sensors / ladder */
      if (ladderChange == -1 && sensorChange == 0) {
        /** case 1 */
        nextSensorID = MPHSensorID;
        // Check for minimum ladderNumber, set to MaxLadder if required
        if (ladderNumber > 1) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          if (layerNumber == 1) {
            nextSensorID.setLadderNumber(8);
          } else if (layerNumber == 2) {
            nextSensorID.setLadderNumber(12);
          }
        }
        uCellDownLeft_tmp = uCells - 1 - uCellsRest;
        uCellUpRight_tmp = uCells - 1;
        vCellDownLeft_tmp = vCellDownLeft;
        vCellUpRight_tmp = vCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == +1 && sensorChange == 0) {
        /** case 2 */
        nextSensorID = MPHSensorID;
        if (layerNumber == 1) {
          if (ladderNumber < 8) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        } else if (layerNumber == 2) {
          if (ladderNumber < 12) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        }
        uCellDownLeft_tmp = 0;
        uCellUpRight_tmp = uCellsRest;
        vCellDownLeft_tmp = vCellDownLeft;
        vCellUpRight_tmp = vCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == 0 && sensorChange == -1) {
        /** case 3 */
        nextSensorID = MPHSensorID;
        nextSensorID.setSensorNumber(1);
        vCellDownLeft_tmp = vCells - 1 - vCellsRest;
        vCellUpRight_tmp = vCells - 1;
        uCellDownLeft_tmp = uCellDownLeft;
        uCellUpRight_tmp = uCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == 0 && sensorChange == +1) {
        /** case 4 */
        nextSensorID = MPHSensorID;
        nextSensorID.setSensorNumber(2);
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;
        uCellDownLeft_tmp = uCellDownLeft;
        uCellUpRight_tmp = uCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == -1 && sensorChange == -1) {
        /** case 5 */
        nextSensorID = MPHSensorID;
        // Check for minimum ladderNumber, set to MaxLadder if required
        if (ladderNumber > 1) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          if (layerNumber == 1) {
            nextSensorID.setLadderNumber(8);
          } else if (layerNumber == 2) {
            nextSensorID.setLadderNumber(12);
          }
        }
        uCellDownLeft_tmp = uCells - 1 - uCellsRest;
        uCellUpRight_tmp = uCells - 1;
        vCellDownLeft_tmp = vCellDownLeft;
        vCellUpRight_tmp = vCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        nextSensorID.setSensorNumber(1);
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;
        uCellDownLeft_tmp = uCellDownLeft;
        uCellUpRight_tmp = uCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        // Check for minimum ladderNumber, set to MaxLadder if required
        if (ladderNumber > 1) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          if (layerNumber == 1) {
            nextSensorID.setLadderNumber(8);
          } else if (layerNumber == 2) {
            nextSensorID.setLadderNumber(12);
          }
        }
        nextSensorID.setSensorNumber(1);
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;
        uCellDownLeft_tmp = uCells - 1 - uCellsRest;
        uCellUpRight_tmp = uCells - 1;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == -1 && sensorChange == +1) {
        /** case 6 */
        nextSensorID = MPHSensorID;
        // Check for minimum ladderNumber, set to MaxLadder if required
        if (ladderNumber > 1) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          if (layerNumber == 1) {
            nextSensorID.setLadderNumber(8);
          } else if (layerNumber == 2) {
            nextSensorID.setLadderNumber(12);
          }
        }
        uCellDownLeft_tmp = uCells - 1 - uCellsRest;
        uCellUpRight_tmp = uCells - 1;
        vCellDownLeft_tmp = vCellDownLeft;
        vCellUpRight_tmp = vCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        //nextSensorID.setSensorNumber(sensorNumber + sensorChange);
        nextSensorID.setSensorNumber(2);
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;
        uCellDownLeft_tmp = uCellDownLeft;
        uCellUpRight_tmp = uCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        // Check for minimum ladderNumber, set to MaxLadder if required
        if (ladderNumber > 1) {
          nextSensorID.setLadderNumber(ladderNumber + ladderChange);
        } else {
          if (layerNumber == 1) {
            nextSensorID.setLadderNumber(8);
          } else if (layerNumber == 2) {
            nextSensorID.setLadderNumber(12);
          }
        }
        nextSensorID.setSensorNumber(2);
        uCellDownLeft_tmp = uCells - 1 - uCellsRest;
        uCellUpRight_tmp = uCells - 1;
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == +1 && sensorChange == -1) {
        /** case 7 */
        nextSensorID = MPHSensorID;
        if (layerNumber == 1) {
          if (ladderNumber < 8) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        } else if (layerNumber == 2) {
          if (ladderNumber < 12) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        }
        uCellDownLeft_tmp = 0;
        uCellUpRight_tmp = uCellsRest;
        vCellDownLeft_tmp = vCellDownLeft;
        vCellUpRight_tmp = vCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        nextSensorID.setSensorNumber(1);
        vCellDownLeft_tmp = vCells - 1 - vCellsRest;
        vCellUpRight_tmp = vCells - 1;
        uCellDownLeft_tmp = uCellDownLeft;
        uCellUpRight_tmp = uCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        if (layerNumber == 1) {
          if (ladderNumber < 8) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        } else if (layerNumber == 2) {
          if (ladderNumber < 12) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        }
        nextSensorID.setSensorNumber(1);
        uCellDownLeft_tmp = 0;
        uCellUpRight_tmp = uCellsRest;
        vCellDownLeft_tmp = vCells - 1 - vCellsRest;
        vCellUpRight_tmp = vCells - 1;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      } else if (ladderChange == +1 && sensorChange == +1) {
        /** case 8 */
        nextSensorID = MPHSensorID;
        if (layerNumber == 1) {
          if (ladderNumber < 8) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        } else if (layerNumber == 2) {
          if (ladderNumber < 12) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        }
        uCellDownLeft_tmp = 0;
        uCellUpRight_tmp = uCellsRest;
        vCellDownLeft_tmp = vCellDownLeft;
        vCellUpRight_tmp = vCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        nextSensorID.setSensorNumber(2);
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;
        uCellDownLeft_tmp = uCellDownLeft;
        uCellUpRight_tmp = uCellUpRight;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

        nextSensorID = MPHSensorID;
        if (layerNumber == 1) {
          if (ladderNumber < 8) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        } else if (layerNumber == 2) {
          if (ladderNumber < 12) {
            nextSensorID.setLadderNumber(ladderNumber + ladderChange);
          } else {
            nextSensorID.setLadderNumber(1);
          }
        }
        nextSensorID.setSensorNumber(2);
        uCellDownLeft_tmp = 0;
        uCellUpRight_tmp = uCellsRest;
        vCellDownLeft_tmp = 0;
        vCellUpRight_tmp = vCellsRest;

        m_storeDATCONROIids.appendNew(ROIid(uCellDownLeft_tmp, uCellUpRight_tmp, vCellDownLeft_tmp, vCellUpRight_tmp, nextSensorID));

      }
    }
  }
}

