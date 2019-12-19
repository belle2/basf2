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

  addParam("overrideDB", m_overrideDB, "If set, ROI-filtering settings in DB are overwritten", false);
  addParam("usePXDDataReduction", m_usePXDDataReduction, "enables/disables ROI-filtering if overrideDB=True", false);
}

void PXDdigiFilterModule::initialize()
{

  StoreArray<ROIid> roiIDs;
  roiIDs.isRequired(m_ROIidsName);

  StoreArray<PXDDigit> PXDDigits(m_PXDDigitsName);   /**< The PXDDigits to be filtered */
  PXDDigits.isRequired();
  if (m_PXDDigitsName == m_PXDDigitsInsideROIName) {
    m_selectorIN.registerSubset(PXDDigits);
  } else {
    m_selectorIN.registerSubset(PXDDigits, m_PXDDigitsInsideROIName, DataStore::c_WriteOut);
    m_selectorIN.inheritAllRelations();
  }

  if (m_CreateOutside) {
    if (m_PXDDigitsName == m_PXDDigitsOutsideROIName) {
      m_selectorOUT.registerSubset(PXDDigits);
    } else {
      m_selectorOUT.registerSubset(PXDDigits, m_PXDDigitsOutsideROIName);
      m_selectorOUT.inheritAllRelations();
    }
  }
}

void PXDdigiFilterModule::beginRun()
{
  // reset variables used to enable/disable ROI-filtering
  m_skipEveryNth = -1;
  if (m_roiParameters) {
    m_skipEveryNth = m_roiParameters->getDisableROIforEveryNth();
  } else {
    B2ERROR("No configuration for the current run found");
  }
  m_countNthEvent = 0;
}

void PXDdigiFilterModule::event()
{
  // parameters might also change on a per-event basis
  if (m_roiParameters.hasChanged()) {
    if (m_roiParameters) {
      m_skipEveryNth = m_roiParameters->getDisableROIforEveryNth();
    } else {
      B2ERROR("No configuration for the current run found");
    }
    // and reset counter
    m_countNthEvent = 0;
  }

  if (m_overrideDB) {
    if (m_usePXDDataReduction) {
      filterDigits();
    } else {
      copyDigits();
    }
    return;
  }

  m_countNthEvent++;

  // Data reduction disabled -> simply copy everything
  if (m_skipEveryNth > 0 and m_countNthEvent % m_skipEveryNth == 0) {
    copyDigits();
    m_countNthEvent = 0;

    return;
  }

  // Perform data reduction
  filterDigits();
}

void PXDdigiFilterModule::filterDigits()
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

void PXDdigiFilterModule::copyDigits()
{
  // omitting the variable name; otherwise a warning is produced (un-used variable)
  m_selectorIN.select([](const PXDDigit* /* thePxdDigit */) {return true;});
}


