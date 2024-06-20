/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/roiFinding/svd/SVDShaperDigitFilterModule.h>
#include <tracking/dataobjects/ROIid.h>

#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDShaperDigitFilter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDShaperDigitFilterModule::SVDShaperDigitFilterModule() : Module()
{
  // Set module properties
  setDescription("The module produce a StoreArray of SVDShaperDigit inside the ROIs.");

  // Parameter definitions
  addParam("SVDShaperDigitsName", m_SVDShaperDigitsName, "The name of the StoreArray of SVDShaperDigits to be filtered",
           std::string(""));
  addParam("SVDShaperDigitsInsideROIName", m_SVDShaperDigitsInsideROIName, "The name of the StoreArray of Filtered SVDShaperDigits",
           std::string("SVDShaperDigitsIN"));
  addParam("SVDShaperDigitsOutsideROIName", m_SVDShaperDigitsOutsideROIName, "The name of the StoreArray of Filtered SVDShaperDigits",
           std::string("SVDShaperDigitsOUT"));
  addParam("ROIidsName", m_ROIidsName, "The name of the StoreArray of ROIs", std::string(""));
  addParam("CreateOutside", m_CreateOutside, "Create the StoreArray of PXD pixel outside the ROIs", false);

}

SVDShaperDigitFilterModule::~SVDShaperDigitFilterModule()
{
}

void SVDShaperDigitFilterModule::initialize()
{

  m_SVDShaperDigits.isRequired(m_SVDShaperDigitsName);
  m_ROIs.isRequired(m_ROIidsName);

  StoreArray<SVDShaperDigit> SVDShaperDigits(m_SVDShaperDigitsName);   /**< The SVDShaperDigits to be filtered */
  SVDShaperDigits.isRequired();
  m_selectorIN.registerSubset(SVDShaperDigits, m_SVDShaperDigitsInsideROIName);
  //  m_selectorIN.inheritAllRelations();

  if (m_CreateOutside) {
    m_selectorOUT.registerSubset(SVDShaperDigits, m_SVDShaperDigitsOutsideROIName);
    //    m_selectorOUT.inheritAllRelations();
  }
}


void SVDShaperDigitFilterModule::event()
{

  std::multimap< VxdID, ROIid > ROIids;

  for (auto ROI : m_ROIs)
    ROIids.insert(std::pair<VxdID, ROIid> (ROI.getSensorID(), ROI));

  m_selectorIN.select([ROIids](const SVDShaperDigit * theSVDShaper) {
    auto ROIidsRange = ROIids.equal_range(theSVDShaper->getSensorID()) ;
    for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
      if (theROI->second.Contains(*theSVDShaper))
        return true;

    return false;
  });

  if (m_CreateOutside) {
    m_selectorOUT.select([ROIids](const SVDShaperDigit * theSVDShaper) {
      auto ROIidsRange = ROIids.equal_range(theSVDShaper->getSensorID()) ;
      for (auto theROI = ROIidsRange.first ; theROI != ROIidsRange.second; theROI ++)
        if (theROI->second.Contains(*theSVDShaper))
          return false;

      return true;
    });
  }

}

