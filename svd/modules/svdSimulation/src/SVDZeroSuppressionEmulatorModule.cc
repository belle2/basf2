/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDZeroSuppressionEmulatorModule.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDZeroSuppressionEmulator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDZeroSuppressionEmulatorModule::SVDZeroSuppressionEmulatorModule() : Module()
{
  setDescription("Filters out SVDShaperDigit with only one sample above a certain threshold, by default set at SN > 3.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string(""));
  addParam("ShaperDigitsIN", m_SVDShaperDigitsIN,
           "Kept ShaperDigits collection name, kept", string(""));
  addParam("ShaperDigitsOUT", m_SVDShaperDigitsOUT,
           "ShaperDigits collection name", string(""));
  addParam("SNthreshold", m_cutSN,
           "minimum SN to not be filtered out", float(3));
  addParam("numberOfSamples", m_nSample,
           "number of samples below threshold to filter out strip ", int(1));
  addParam("createOUTStripsList", m_createOutside,
           "create the StoreArray of outside strips", bool(false));

}


SVDZeroSuppressionEmulatorModule::~SVDZeroSuppressionEmulatorModule()
{
}


void SVDZeroSuppressionEmulatorModule::initialize()
{
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  m_selectorIN.registerSubset(m_storeShaper, m_SVDShaperDigitsIN);
  //  m_selectorIN.inheritAllRelations();

  if (m_createOutside) {
    m_selectorOUT.registerSubset(m_storeShaper, m_SVDShaperDigitsOUT);
    //    m_selectorOUT.inheritAllRelations();
  }

}


void SVDZeroSuppressionEmulatorModule::event()
{

  // If no digits, nothing to do
  if (!m_storeShaper || !m_storeShaper.getEntries()) return;

  m_selectorIN.select([&](const SVDShaperDigit * shaper) { return ! this->passesZS(shaper) ;});

  if (m_createOutside)
    m_selectorOUT.select([&](const SVDShaperDigit * shaper) { return this->passesZS(shaper) ;});

  B2DEBUG(10, "     shaper digits = " << m_storeShaper.getEntries() <<
          ", shaper digits IN = " << (((StoreArray<SVDShaperDigit>*)(m_selectorIN.getSubSet()))->getEntries()));

  if (m_createOutside)
    B2DEBUG(10, "     shaper digits = " << m_storeShaper.getEntries() <<
            ", shaper digits IN = " << (((StoreArray<SVDShaperDigit>*)(m_selectorIN.getSubSet()))->getEntries()) <<
            ", shaper digits OUT = " << (((StoreArray<SVDShaperDigit>*)(m_selectorOUT.getSubSet()))->getEntries()));

}


bool SVDZeroSuppressionEmulatorModule::passesZS(const SVDShaperDigit* shaper)
{

  VxdID thisSensorID = shaper->getSensorID();
  bool thisSide = shaper->isUStrip();
  int thisCellID = shaper->getCellID();

  float noise = m_NoiseCal.getNoise(thisSensorID, thisSide, thisCellID);
  float cutMinSignal = m_cutSN * noise;

  if (shaper->passesZS(m_nSample, cutMinSignal))
    return true;

  return false;
};

