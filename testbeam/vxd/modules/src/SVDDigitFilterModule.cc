/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/SVDDigitFilterModule.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDDigit.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigitFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigitFilterModule::SVDDigitFilterModule() : Module()
{
  // Set module properties
  setDescription("Performs the easiest possible preselection of SVD digits");

  std::vector<int> maskU;
  maskU.push_back(-1);
  std::vector<int> maskV;
  maskV.push_back(-1);
  // Parameter definitions
  addParam("maskUupTo", m_maskUupTo, "All U-digits bellow this will not pass through the filter", 0);
  addParam("maskUfrom", m_maskUfrom, "All U-digits after this will not pass through the filter", 768);
  addParam("maskVupTo", m_maskVupTo, "All V-digits bellow this will not pass through the filter", 0);
  addParam("maskVfrom", m_maskVfrom, "All V-digits after this will not pass through the filter", 768);
  addParam("maskStripsU", m_maskStripsU, "Stated U-digit strips will not pass through the filter", maskU);
  addParam("maskStripsV", m_maskStripsV, "Stated V-digit strips will not pass through the filter", maskV);
  addParam("inputDigits", m_inputDigits, "Input SVD digits for filtering collection name", std::string("SVDDigitsNotFiltered"));
  addParam("outputDigits", m_outputDigits, "Output SVD digits collection name", std::string("SVDDigits"));
  addParam("sensorID", m_sensorID, "Sensor VxdID to mask", std::string("6.1.6"));


}

SVDDigitFilterModule::~SVDDigitFilterModule()
{
}

void SVDDigitFilterModule::initialize()
{
  StoreArray<SVDDigit> required(m_inputDigits);
  StoreArray<SVDDigit> storeOutputDigits(m_outputDigits);
  storeOutputDigits.registerInDataStore();
}

void SVDDigitFilterModule::beginRun()
{
}

void SVDDigitFilterModule::event()
{
  StoreArray<SVDDigit> oldDigits(m_inputDigits);
  StoreArray<SVDDigit> newDigits(m_outputDigits);
  for (auto digit : oldDigits) {
    if (digit.getSensorID() != VxdID(m_sensorID))
      continue;
    if (digit.isUStrip()) {
      // u strip
      if (digit.getCellID() < m_maskUupTo) {
        // do nothing
      } else if (digit.getCellID() > m_maskUfrom) {
        // do nothing

      } else if (std::find(m_maskStripsU.begin(), m_maskStripsU.end(), digit.getCellID()) != m_maskStripsU.end()) {
        // do nothing
      } else {
        // store filetered digits in the new collection
        newDigits.appendNew(SVDDigit(digit));
      }

    } else {
      // v strip
      if (digit.getCellID() < m_maskVupTo) {
        // do nothing
      } else if (digit.getCellID() > m_maskVfrom) {
        // do nothing
      } else if (std::find(m_maskStripsV.begin(), m_maskStripsV.end(), digit.getCellID()) != m_maskStripsV.end()) {
        // do nothing
      } else {
        // store filetered digits in the new collection
        newDigits.appendNew(SVDDigit(digit));
      }
    }
  }
}

void SVDDigitFilterModule::endRun()
{
}

void SVDDigitFilterModule::terminate()
{
}


