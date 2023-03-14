/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDRawHitFilterModule.h>
#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawHitFilter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawHitFilterModule::PXDRawHitFilterModule() : Module()
{
  // Set module properties
  setDescription("The module produce a StoreArray of PXDRawHit inside the ROIs.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of PXDRawHits to be filtered", std::string(""));
  addParam("PXDRawHitsInsideROIName", m_PXDRawHitsInsideROIName, "The name of the StoreArray of Filtered PXDRawHits",
           std::string("PXDRawHitsIN"));
  addParam("PXDRawHitsOutsideROIName", m_PXDRawHitsOutsideROIName, "The name of the StoreArray of Filtered PXDRawHits",
           std::string("PXDRawHitsOUT"));
  addParam("ROIidsName", m_ROIidsName, "The name of the StoreArray of ROIs", std::string(""));
  addParam("CreateOutside", m_CreateOutside, "Create the StoreArray of PXD pixel outside the ROIs", false);

}

void PXDRawHitFilterModule::initialize()
{
  m_ROIs.isRequired(m_ROIidsName);
  m_PXDRawHits.isRequired(m_PXDRawHitsName);   /**< The PXDRawHits to be filtered */

  if (m_PXDRawHitsName == m_PXDRawHitsInsideROIName) {
    m_selectorIN.registerSubset(m_PXDRawHits);
  } else {
    m_selectorIN.registerSubset(m_PXDRawHits, m_PXDRawHitsInsideROIName);
    m_selectorIN.inheritAllRelations();
  }

  if (m_CreateOutside) {
    if (m_PXDRawHitsName == m_PXDRawHitsOutsideROIName) {
      m_selectorOUT.registerSubset(m_PXDRawHits);
    } else {
      m_selectorOUT.registerSubset(m_PXDRawHits, m_PXDRawHitsOutsideROIName);
      m_selectorOUT.inheritAllRelations();
    }
  }
}

void PXDRawHitFilterModule::event()
{
  std::multimap< VxdID, ROIid > ROIids;

  for (auto ROI : m_ROIs)
    ROIids.insert(std::pair<VxdID, ROIid> (ROI.getSensorID(), ROI));

  m_selectorIN.select([ROIids](const PXDRawHit * thePxdRawHit) {
    auto ROIidsRange = ROIids.equal_range(thePxdRawHit->getSensorID()) ;
    for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
      if (theROI->second.Contains(*thePxdRawHit))
        return true;

    return false;
  });

  if (m_CreateOutside) {
    m_selectorOUT.select([ROIids](const PXDRawHit * thePxdRawHit) {
      auto ROIidsRange = ROIids.equal_range(thePxdRawHit->getSensorID()) ;
      for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
        if (theROI->second.Contains(*thePxdRawHit))
          return false;

      return true;
    });
  }

}
