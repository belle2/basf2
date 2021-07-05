/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/datcon/findlets/ROICalculator.h>
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

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ROIsStoreArrayName"), m_param_ROIsStoreArrayName,
                                "Name of the ROIs StoreArray?", m_param_ROIsStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "uROIsizeL1"), m_param_uROIsizeL1,
                                "u direction ROI size on L1. Data type: short", m_param_uROIsizeL1);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "uROIsizeL2"), m_param_uROIsizeL2,
                                "u direction ROI size on L2. Data type: short", m_param_uROIsizeL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vROIsizeL1"), m_param_vROIsizeL1,
                                "v direction ROI size on L1. Data type: short", m_param_vROIsizeL1);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vROIsizeL2"), m_param_vROIsizeL2,
                                "v direction ROI size on L2. Data type: short", m_param_vROIsizeL2);

}

void ROICalculator::initialize()
{
  Super::initialize();

  m_storeDATCONROIs.registerInDataStore(m_param_ROIsStoreArrayName);
}

void ROICalculator::apply(const std::vector<std::pair<VxdID, long>>& uExtrapolations,
                          const std::vector<std::pair<VxdID, long>>& vExtrapolations)
{
  /** Reminder: 250 px in u-direction = r-phi, in total 768 (512+256) px in v-direction = z */
  const unsigned short uCells = 250, vCells = 768;

  const PXD::SensorInfo* currentSensor;

  for (auto& uExtrapolatedHit : uExtrapolations) {
    const VxdID& uHitSensorID = uExtrapolatedHit.first;

    for (auto& vExtrapolatedHit : vExtrapolations) {
      const VxdID& vHitSensorID = vExtrapolatedHit.first;

      if (uHitSensorID != vHitSensorID) {
        continue;
      }

      // Convert back from nm to cm.
      // Before the all values were upscaled with "convertFloatToInt" as the FPGA only works with integers.
      // This conversion from nm to cm is hardcoded, if the powers of 10 are changed in the other modules, this conversion
      // would not necessarily be right anymore, so maybe there is a better way of doing this - I didn't find any.
      double uCoordinateInCM = uExtrapolatedHit.second * Unit::nm;
      double vCoordinateInCM = vExtrapolatedHit.second * Unit::nm;

      currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(uHitSensorID));
      const short uCell = currentSensor->getUCellID(uCoordinateInCM);
      const short vCell = currentSensor->getVCellID(vCoordinateInCM);

      const unsigned short& uROIsize = (uHitSensorID.getLayerNumber() == 1 ? m_param_uROIsizeL1 : m_param_uROIsizeL2);
      const unsigned short& vROIsize = (uHitSensorID.getLayerNumber() == 1 ? m_param_vROIsizeL1 : m_param_vROIsizeL2);

      B2DEBUG(29, "sensorID: " << uHitSensorID << ", localUCoordinate: " << uCoordinateInCM << ", localVCoordinate: " << vCoordinateInCM
              << ", localUCell: " << uCell << ", localVCell: " << vCell << ", uROIsize : " << uROIsize << ", vROIsize: " << vROIsize);

      /** Lower left corner */
      short uCellDownLeft = uCell - uROIsize / 2;
      if (uCellDownLeft < 0) uCellDownLeft = 0;
      short vCellDownLeft = vCell - vROIsize / 2;
      if (vCellDownLeft < 0) vCellDownLeft = 0;

      /** Upper right corner */
      short uCellUpRight = uCell + uROIsize / 2;
      if (uCellUpRight >= uCells) uCellUpRight = uCells - 1;
      short vCellUpRight = vCell + vROIsize / 2;
      if (vCellUpRight >= vCells) vCellUpRight = vCells - 1;

      m_storeDATCONROIs.appendNew(ROIid(uCellDownLeft, uCellUpRight, vCellDownLeft, vCellUpRight, uHitSensorID));
    }
  }
}
