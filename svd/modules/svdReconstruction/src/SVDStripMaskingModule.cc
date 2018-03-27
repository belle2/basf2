/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDStripMaskingModule.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDStripMasking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDStripMaskingModule::SVDStripMaskingModule() : Module()
{
  setDescription("Remove from the SVDShaperDigit list the hot strips read in the SVDHotStripsCalibrations from the DB.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string(""));
  addParam("ShaperDigitsUnmasked", m_SVDShaperDigitsUnmasked,
           "Good ShaperDigits collection name, kept", string(""));
  addParam("ShaperDigitsMasked", m_SVDShaperDigitsMasked,
           "ShaperDigits collection name", string(""));
  addParam("createHotStripsList", m_createOutside,
           "create the StoreArray of hot strips", bool(false));

}


SVDStripMaskingModule::~SVDStripMaskingModule()
{
}


void SVDStripMaskingModule::initialize()
{
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  m_selectorIN.registerSubset(m_storeShaper, m_SVDShaperDigitsUnmasked);
  //  m_selectorIN.inheritAllRelations();

  if (m_createOutside) {
    m_selectorOUT.registerSubset(m_storeShaper, m_SVDShaperDigitsMasked);
    //    m_selectorOUT.inheritAllRelations();
  }

}


void SVDStripMaskingModule::event()
{

  // If no digits, nothing to do
  if (!m_storeShaper || !m_storeShaper.getEntries()) return;

  m_selectorIN.select([&](const SVDShaperDigit * shaper) { return ! m_HotStripsCalib.isHot(shaper->getSensorID(), shaper->isUStrip(), shaper->getCellID());});

  if (m_createOutside)
    m_selectorOUT.select([&](const SVDShaperDigit * shaper) { return m_HotStripsCalib.isHot(shaper->getSensorID(), shaper->isUStrip(), shaper->getCellID());});

  B2DEBUG(10, "     shaper digits = " << m_storeShaper.getEntries() <<
          ", shaper digits Unmasked = " << (((StoreArray<SVDShaperDigit>*)(m_selectorIN.getSubSet()))->getEntries()));

  if (m_createOutside)
    B2DEBUG(10, "     shaper digits = " << m_storeShaper.getEntries() <<
            ", shaper digits Unmasked = " << (((StoreArray<SVDShaperDigit>*)(m_selectorIN.getSubSet()))->getEntries()) <<
            ", shaper digits Masked = " << (((StoreArray<SVDShaperDigit>*)(m_selectorOUT.getSubSet()))->getEntries()));

}



