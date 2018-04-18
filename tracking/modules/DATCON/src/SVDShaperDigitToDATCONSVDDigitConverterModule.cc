/******************************************************************************
* BASF2 (Belle Analysis Framework 2)                                          *
* Copyright(C) 2010 - Belle II Collaboration                                  *
*                                                                             *
* Author: The Belle II Collaboration                                          *
* Contributors: Christian Wessel                                              *
*                                                                             *
* This software is provided "as is" without any warranty.                     *
* Beware! Do not expose to open flames it can explode                         *
******************************************************************************/

#include <tracking/modules/DATCON/SVDShaperDigitToDATCONSVDDigitConverterModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDShaperDigitToDATCONSVDDigitConverter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDShaperDigitToDATCONSVDDigitConverterModule::SVDShaperDigitToDATCONSVDDigitConverterModule() : Module()
{
  //Set module properties
  setDescription("Converts SVDShaperDigits into DATCONSVDDigits and registers the relations needed.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("svdShaperDigitListName", m_storeSVDShaperDigitsListName, "Name of the SVDShaperDigits list", string(""));
  addParam("DATCONSVDDigitListName", m_storeDATCONSVDDigitsListName, "Name of the m_storeDATCONSVDDigits list", string(""));
  addParam("svdTrueHitsListName", m_storeTrueHitsListName, "Name of the SVDTrueHit list", string(""));
  addParam("mcParticlesListName", m_storeMCParticlesListName, "Name of the MCParticles list", string(""));
}


void SVDShaperDigitToDATCONSVDDigitConverterModule::initialize()
{
  m_storeDATCONSVDDigits.registerInDataStore(m_storeDATCONSVDDigitsListName, DataStore::c_DontWriteOut);
  m_storeDATCONSVDDigitsListName = m_storeDATCONSVDDigits.getName();

  m_storeSVDShaperDigits.isRequired(m_storeSVDShaperDigitsListName);
  m_storeSVDShaperDigitsListName = m_storeSVDShaperDigits.getName();

  m_storeDATCONSVDDigits.registerRelationTo(m_storeSVDShaperDigits);

  m_storeTrueHits.isOptional(m_storeTrueHitsListName);
  if (m_storeTrueHits.isValid()) {
    m_storeTrueHitsListName = m_storeTrueHits.getName();
    m_storeDATCONSVDDigits.registerRelationTo(m_storeTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  m_storeMCParticles.isOptional(m_storeMCParticlesListName);
  if (m_storeMCParticles.isValid()) {
    m_storeMCParticlesListName = m_storeMCParticles.getName();
    m_storeDATCONSVDDigits.registerRelationTo(m_storeMCParticles, DataStore::c_Event, DataStore::c_DontWriteOut);
  }
}


void SVDShaperDigitToDATCONSVDDigitConverterModule::event()
{
  for (auto& shaperdigit : m_storeSVDShaperDigits) {

    RelationVector<MCParticle> relatedMC = shaperdigit.getRelationsTo<MCParticle>();
    RelationVector<SVDTrueHit> relatedSVDTrue = shaperdigit.getRelationsTo<SVDTrueHit>();
    VxdID sensorid = shaperdigit.getSensorID();
    bool isu = shaperdigit.isUStrip();
    short cellid = shaperdigit.getCellID();
    DATCONSVDDigit::APVFloatSamples rawsamples = shaperdigit.getSamples();
    SVDModeByte mode = shaperdigit.getModeByte();

    DATCONSVDDigit newDATCONSVDDigit(sensorid, isu, cellid, rawsamples, mode);
    DATCONSVDDigit* datcondigit = m_storeDATCONSVDDigits.appendNew(newDATCONSVDDigit);

    // Register relation to the SVDShaperDigit the DATCONSVDDigit is derived from
    datcondigit->addRelationTo(&shaperdigit);

    // Register relations to the according MCParticle and SVDTrueHit
    if (relatedMC.size() > 0) {
      for (unsigned int relmcindex = 0; relmcindex < relatedMC.size(); relmcindex++) {
        datcondigit->addRelationTo(relatedMC[relmcindex], relatedMC.weight(relmcindex));
      }
    }
    if (relatedSVDTrue.size() > 0) {
      for (unsigned int reltruehitindex = 0; reltruehitindex < relatedSVDTrue.size(); reltruehitindex++) {
        datcondigit->addRelationTo(relatedSVDTrue[reltruehitindex], relatedSVDTrue.weight(reltruehitindex));
      }
    }

  }

} //end event function

