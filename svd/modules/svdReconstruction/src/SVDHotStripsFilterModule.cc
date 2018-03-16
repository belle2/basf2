/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDHotStripsFilterModule.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDHotStripsFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDHotStripsFilterModule::SVDHotStripsFilterModule() : Module()
{
  setDescription("Filters out SVDShaperDigit with only one sample above a certain threshold, by default set at SN > 3.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string(""));
  addParam("ShaperDigitsGOOD", m_SVDShaperDigitsGOOD,
           "Good ShaperDigits collection name, kept", string(""));
  addParam("ShaperDigitsHOT", m_SVDShaperDigitsHOT,
           "ShaperDigits collection name", string(""));
  addParam("SNthreshold", m_cutSN,
           "minimum SN to not be filtered out", float(3));
  addParam("numberOfSamples", m_nSample,
           "max number of samples above threshod to mark the strip as hot", int(1));
  addParam("createHotStripsList", m_createOutside,
           "create the StoreArray of hot strips", bool(false));

}


SVDHotStripsFilterModule::~SVDHotStripsFilterModule()
{
}


void SVDHotStripsFilterModule::initialize()
{
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  m_selectorIN.registerSubset(m_storeShaper, m_SVDShaperDigitsGOOD);
  //  m_selectorIN.inheritAllRelations();

  if (m_createOutside) {
    m_selectorOUT.registerSubset(m_storeShaper, m_SVDShaperDigitsHOT);
    //    m_selectorOUT.inheritAllRelations();
  }

}


void SVDHotStripsFilterModule::event()
{

  // If no digits, nothing to do
  if (!m_storeShaper || !m_storeShaper.getEntries()) return;

  m_selectorIN.select([&](const SVDShaperDigit * shaper) { return ! this->isHot(shaper) ;});

  if (m_createOutside)
    m_selectorOUT.select([&](const SVDShaperDigit * shaper) { return this->isHot(shaper) ;});

  B2DEBUG(10, "     shaper digits = " << m_storeShaper.getEntries() <<
          ", shaper digits GOOD = " << (((StoreArray<SVDShaperDigit>*)(m_selectorIN.getSubSet()))->getEntries()));

  if (m_createOutside)
    B2DEBUG(10, "     shaper digits = " << m_storeShaper.getEntries() <<
            ", shaper digits GOOD = " << (((StoreArray<SVDShaperDigit>*)(m_selectorIN.getSubSet()))->getEntries()) <<
            ", shaper digits HOT = " << (((StoreArray<SVDShaperDigit>*)(m_selectorOUT.getSubSet()))->getEntries()));

}


bool SVDHotStripsFilterModule::isHot(const SVDShaperDigit* shaper)
{

  VxdID thisSensorID = shaper->getSensorID();
  bool thisSide = shaper->isUStrip();
  int thisCellID = shaper->getCellID();

  float noise = m_NoiseCal.getNoise(thisSensorID, thisSide, thisCellID);
  float cutMinSignal = m_cutSN * noise;

  if (shaper->isHot(m_nSample, cutMinSignal))
    return true;

  return false;
};

