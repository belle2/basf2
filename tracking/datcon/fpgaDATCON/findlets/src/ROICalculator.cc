/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/ROICalculator.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/gearbox/Unit.h>
#include <tracking/dataobjects/ROIid.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ROICalculator::ROICalculator() : Super()
{
}

void ROICalculator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ROIsStoreArrayName"),
                                m_param_ROIsStoreArrayName,
                                "Name of the ROIs StoreArray?",
                                m_param_ROIsStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "uROIsizeL1"),
                                m_param_uROIsizeL1,
                                "u direction ROI size on L1.",
                                m_param_uROIsizeL1);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "uROIsizeL2"),
                                m_param_uROIsizeL2,
                                "u direction ROI size on L2.",
                                m_param_uROIsizeL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vROIsizeL1"),
                                m_param_vROIsizeL1,
                                "v direction ROI size on L1.",
                                m_param_vROIsizeL1);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vROIsizeL2"),
                                m_param_vROIsizeL2,
                                "v direction ROI size on L2.",
                                m_param_vROIsizeL2);

}

void ROICalculator::initialize()
{
  Super::initialize();

  m_storeDATCONROIs.registerInDataStore(m_param_ROIsStoreArrayName);
  m_param_ROIsStoreArrayName = m_storeDATCONROIs.getName();

}

void ROICalculator::apply(std::vector<std::pair<VxdID, long>>& uExtrapolations,
                          std::vector<std::pair<VxdID, long>>& vExtrapolations)
{
  /** Reminder: 250 px in u-direction = r-phi, in total 768 (512+256) px in v-direction = z */
//   double uCoordinate, vCoordinate;
  const unsigned short uCells = 250, vCells = 768;

  const PXD::SensorInfo* currentSensor;

  for (auto& uExtrapolatedHit : uExtrapolations) {
    const VxdID& uHitSensorID = uExtrapolatedHit.first;

    for (auto& vExtrapolatedHit : vExtrapolations) {
      const VxdID& vHitSensorID = vExtrapolatedHit.first;

      if (uHitSensorID != vHitSensorID) {
        continue;
      }
//       uCoordinate = datconumph.getLocalCoordinate().X(); // in nm
//       vCoordinate = datconvmph.getLocalCoordinate().Y(); // in nm

      // convert back from nm to cm
      double uCoordinateInCM = uExtrapolatedHit.second * Unit::nm;
      double vCoordinateInCM = vExtrapolatedHit.second * Unit::nm;

//       B2INFO("uCoordinateInCM: " << uCoordinateInCM << " vCoordinateInCM: " << vCoordinateInCM);

      currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(uHitSensorID));
      const short uCell = currentSensor->getUCellID(uCoordinateInCM);
      const short vCell = currentSensor->getVCellID(vCoordinateInCM);

      const unsigned short& uROIsize = (uHitSensorID.getLayerNumber() == 1 ? m_param_uROIsizeL1 : m_param_uROIsizeL2);
      const unsigned short& vROIsize = (uHitSensorID.getLayerNumber() == 1 ? m_param_vROIsizeL1 : m_param_vROIsizeL2);

//       B2INFO("sensorID: " << uHitSensorID << ", localUCoordinate: " << uCoordinate << ", localVCoordinate: " << vCoordinate <<
//              ", localUCell: " << uCell << ", localVCell: " << vCell << ", uROIsize : " << uROIsize << ", vROIsize: " << vROIsize);

      /** Lower left corner */
      short uCellDownLeft = uCell - uROIsize / 2;
      short vCellDownLeft = vCell - vROIsize / 2;
      if (vCellDownLeft < 0) vCellDownLeft = 0;
      if (uCellDownLeft < 0) uCellDownLeft = 0;

      /** Upper right corner */
      short uCellUpRight = uCell + uROIsize / 2;
      short vCellUpRight = vCell + vROIsize / 2;
      if (vCellUpRight >= vCells) vCellUpRight = vCells - 1;
      if (uCellUpRight >= uCells) uCellUpRight = uCells - 1;

      m_storeDATCONROIs.appendNew(ROIid(uCellDownLeft, uCellUpRight, vCellDownLeft, vCellUpRight, uHitSensorID));
//      if (storeDATCONROIids.getEntries() > 1000) {
//        B2ERROR("Too many DATCON ROIs, aborting ROI creation.");
//        break;
//      }
    }
//    if (storeDATCONROIids.getEntries() > 1000) {
//      storeDATCONROIids.clear();
//      break;
//    }
  }
}
