/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDZeroSuppressionEmulatorModule.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDZeroSuppressionEmulator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDZeroSuppressionEmulatorModule::SVDZeroSuppressionEmulatorModule() : Module()
{
  setDescription("Filters out SVDShaperDigit with less than 1 (default) sample is below a certain threshold, by default set at SN = 3.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("FADCmode", m_FADCmode,
           "FADC mode: if true the approximation to integer is done", bool(true));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string(""));
  addParam("ShaperDigitsIN", m_SVDShaperDigitsIN,
           "Kept ShaperDigits collection name", string(""));
  addParam("ShaperDigitsOUT", m_SVDShaperDigitsOUT,
           "ShaperDigits collection name", string(""));
  addParam("SNthreshold", m_cutSN,
           "minimum SN to be kept", float(3));
  addParam("numberOfSamples", m_nSample,
           "number of samples above threshold to be kept ", int(1));
  addParam("createOUTStripsList", m_createOutside,
           "create the StoreArray of outside strips", bool(false));


}


SVDZeroSuppressionEmulatorModule::~SVDZeroSuppressionEmulatorModule()
{
}


void SVDZeroSuppressionEmulatorModule::initialize()
{
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  if (m_storeShaperDigitsName == m_SVDShaperDigitsIN) {
    m_selectorIN.registerSubset(m_storeShaper);
  } else {
    m_selectorIN.registerSubset(m_storeShaper, m_SVDShaperDigitsIN, DataStore::c_WriteOut);
    m_selectorIN.inheritAllRelations();
  }

  if (m_createOutside) {
    if (m_storeShaperDigitsName == m_SVDShaperDigitsOUT) {
      m_selectorOUT.registerSubset(m_storeShaper);
    } else {
      m_selectorOUT.registerSubset(m_storeShaper, m_SVDShaperDigitsOUT);
      m_selectorOUT.inheritAllRelations();
    }
  }

}


void SVDZeroSuppressionEmulatorModule::event()
{

  // If no digits, nothing to do
  if (!m_storeShaper || !m_storeShaper.getEntries()) return;

  m_selectorIN.select([&](const SVDShaperDigit * shaper) { return this->passesZS(shaper) ;});

  if (m_createOutside)
    m_selectorOUT.select([&](const SVDShaperDigit * shaper) { return ! this->passesZS(shaper) ;});

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

  //  B2INFO("before = "<<cutMinSignal);
  if (m_FADCmode) {
    cutMinSignal = cutMinSignal + 0.5;
    cutMinSignal = (int)cutMinSignal;
  }
  //  B2INFO("after = "<<cutMinSignal);
  if (shaper->passesZS(m_nSample, cutMinSignal))
    return true;

  return false;
};

