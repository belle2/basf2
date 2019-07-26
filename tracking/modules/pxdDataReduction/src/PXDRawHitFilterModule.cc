/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Paoloni                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDRawHitFilterModule.h>
#include <tracking/dataobjects/ROIid.h>
#include <map>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawHitFilter)

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

  StoreArray<ROIid> roiIDs;
  roiIDs.isRequired(m_ROIidsName);

  StoreArray<PXDRawHit> PXDRawHits(m_PXDRawHitsName);   /**< The PXDRawHits to be filtered */
  PXDRawHits.isRequired();
  if (m_PXDRawHitsName == m_PXDRawHitsInsideROIName) {
    m_selectorIN.registerSubset(PXDRawHits);
  } else {
    m_selectorIN.registerSubset(PXDRawHits, m_PXDRawHitsInsideROIName);
    m_selectorIN.inheritAllRelations();
  }

  if (m_CreateOutside) {
    if (m_PXDRawHitsName == m_PXDRawHitsOutsideROIName) {
      m_selectorOUT.registerSubset(PXDRawHits);
    } else {
      m_selectorOUT.registerSubset(PXDRawHits, m_PXDRawHitsOutsideROIName);
      m_selectorOUT.inheritAllRelations();
    }
  }
}

void PXDRawHitFilterModule::event()
{
  StoreArray<PXDRawHit> PXDRawHits(m_PXDRawHitsName);   /**< The PXDRawHits to be filtered */
  StoreArray<ROIid> ROIids_store_array(m_ROIidsName); /**< The ROIs */

  multimap< VxdID, ROIid > ROIids;

  for (auto ROI : ROIids_store_array)
    ROIids.insert(pair<VxdID, ROIid> (ROI.getSensorID() , ROI));

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
