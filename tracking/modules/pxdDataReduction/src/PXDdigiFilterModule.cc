/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Paoloni                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDdigiFilterModule.h>
#include <tracking/dataobjects/ROIid.h>
#include <map>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDdigiFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDdigiFilterModule::PXDdigiFilterModule() : Module()
{
  // Set module properties
  setDescription("The module produce a StoreArray of PXDDigit inside the ROIs.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("PXDDigitsName", m_PXDDigitsName, "The name of the StoreArray of PXDDigits to be filtered", std::string(""));
  addParam("PXDDigitsInsideROIName", m_PXDDigitsInsideROIName, "The name of the StoreArray of Filtered PXDDigits",
           std::string("PXDDigitsIN"));
  addParam("PXDDigitsOutsideROIName", m_PXDDigitsOutsideROIName, "The name of the StoreArray of Filtered PXDDigits",
           std::string("PXDDigitsOUT"));
  addParam("ROIidsName", m_ROIidsName, "The name of the StoreArray of ROIs", std::string(""));
  addParam("CreateOutside", m_CreateOutside, "Create the StoreArray of PXD pixel outside the ROIs", false);

}

PXDdigiFilterModule::~PXDdigiFilterModule()
{
}

void PXDdigiFilterModule::initialize()
{

  StoreArray<ROIid> roiIDs;
  roiIDs.isRequired(m_ROIidsName);

  StoreArray<PXDDigit> PXDDigits(m_PXDDigitsName);   /**< The PXDDigits to be filtered */
  PXDDigits.isRequired();
  m_selectorIN.registerSubset(PXDDigits, m_PXDDigitsInsideROIName);
  m_selectorIN.inheritAllRelations();

  if (m_CreateOutside) {
    m_selectorOUT.registerSubset(PXDDigits, m_PXDDigitsOutsideROIName);
    m_selectorOUT.inheritAllRelations();
  }
}

void PXDdigiFilterModule::beginRun()
{
}

void PXDdigiFilterModule::event()
{
  StoreArray<PXDDigit> PXDDigits(m_PXDDigitsName);   /**< The PXDDigits to be filtered */
  StoreArray<ROIid> ROIids_store_array(m_ROIidsName); /**< The ROIs */

  multimap< VxdID, ROIid > ROIids;

  for (auto ROI : ROIids_store_array)
    ROIids.insert(pair<VxdID, ROIid> (ROI.getSensorID() , ROI));

  m_selectorIN.select([ROIids](const PXDDigit * thePxdDigit) {
    auto ROIidsRange = ROIids.equal_range(thePxdDigit->getSensorID()) ;
    for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
      if (theROI->second.Contains(*thePxdDigit))
        return true;

    return false;
  });

  if (m_CreateOutside) {
    m_selectorOUT.select([ROIids](const PXDDigit * thePxdDigit) {
      auto ROIidsRange = ROIids.equal_range(thePxdDigit->getSensorID()) ;
      for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
        if (theROI->second.Contains(*thePxdDigit))
          return false;

      return true;
    });
  }

}

void PXDdigiFilterModule::endRun()
{
}

void PXDdigiFilterModule::terminate()
{
}
