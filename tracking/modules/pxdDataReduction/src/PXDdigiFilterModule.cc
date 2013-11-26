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
#include <pxd/dataobjects/PXDDigit.h>
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

  // Parameter definitions
  addParam("PXDDigitsName", m_PXDDigitsName, "The name of the StoreArray of PXDDigits to be filtered", std::string(""));
  addParam("PXDFilteredDigitsName", m_PXDFilteredDigitsName, "The name of the StoreArray of Filtered PXDDigits", std::string("PXDDigitsInsideROIs"));
  addParam("ROIidsName", m_ROIidsName, "The name of the StoreArray of ROIs", std::string(""));

}

PXDdigiFilterModule::~PXDdigiFilterModule()
{
}

void PXDdigiFilterModule::initialize()
{

  StoreArray<PXDDigit>::required(m_PXDDigitsName);
  StoreArray<ROIid>::required(m_ROIidsName);

  StoreArray<PXDDigit>::registerPersistent(m_PXDFilteredDigitsName);
}

void PXDdigiFilterModule::beginRun()
{
}

void PXDdigiFilterModule::event()
{
  StoreArray<PXDDigit> PXDDigits(m_PXDDigitsName);   /**< The PXDDigits to be filtered */
  StoreArray<ROIid> ROIids_store_array(m_ROIidsName); /**< The ROIs */

  StoreArray<PXDDigit> PXDFilteredDigits(m_PXDFilteredDigitsName);   /**< The PXDDigits in ROIs*/
  PXDFilteredDigits.create();

  multimap< VxdID, ROIid > ROIids;

  for (auto ROI : ROIids_store_array)
    ROIids.insert(pair<VxdID, ROIid> (ROI.getSensorID() , ROI));

  for (auto thePxdDigit : PXDDigits) {
    auto ROIidsRange = ROIids.equal_range(thePxdDigit.getSensorID()) ;
    for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
      if (theROI->second.Contains(thePxdDigit)) {
        PXDFilteredDigits.appendNew(thePxdDigit);
        break;
      }


  }


}

void PXDdigiFilterModule::endRun()
{
}

void PXDdigiFilterModule::terminate()
{
}


