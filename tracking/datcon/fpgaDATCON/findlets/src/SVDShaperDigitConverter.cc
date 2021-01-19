/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/SVDShaperDigitConverter.h>

#include <mdst/dataobjects/MCParticle.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>

#include <tracking/dataobjects/DATCONSVDDigit.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SVDShaperDigitConverter::SVDShaperDigitConverter() : Super()
{
//   this->addProcessingSignalListener(&m_trackFitter);
}

void SVDShaperDigitConverter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
//   m_trackFitter.exposeParameters(moduleParamList, prefix);

//   Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeSVDShaperDigitsName"),
                                m_param_storeSVDShaperDigitsName,
                                "StoreArray name of the SVDShaperDigits StoreArray.",
                                m_param_storeSVDShaperDigitsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeDATCONSVDDigitsName"),
                                m_param_storeDATCONSVDDigitsName,
                                "StoreArray name of the DATCONSVDDigits StoreArray.",
                                m_param_storeDATCONSVDDigitsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeTrueHitsName"),
                                m_param_storeTrueHitsName,
                                "StoreArray name of the SVDTrueHits StoreArray.",
                                m_param_storeTrueHitsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeMCParticlesName"),
                                m_param_storeMCParticlesName,
                                "StoreArray name of the MCParticles StoreArray.",
                                m_param_storeMCParticlesName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "saveRelationsInDigitConversion"),
                                m_param_saveStoreArrays,
                                "Save relations of DATCONSVDDigits to MCParticles and SVDTrueHits?.",
                                m_param_saveStoreArrays);

}

void SVDShaperDigitConverter::initialize()
{
  Super::initialize();

  m_storeSVDShaperDigits.isRequired(m_param_storeSVDShaperDigitsName);
  m_param_storeSVDShaperDigitsName = m_storeSVDShaperDigits.getName();

  if (m_param_saveStoreArrays) {

    m_storeDATCONSVDDigits.registerInDataStore(m_param_storeDATCONSVDDigitsName, DataStore::c_DontWriteOut);
    m_param_storeDATCONSVDDigitsName = m_storeDATCONSVDDigits.getName();
    m_storeDATCONSVDDigits.registerRelationTo(m_storeSVDShaperDigits);

    m_storeTrueHits.isOptional(m_param_storeTrueHitsName);
    if (m_storeTrueHits.isValid()) {
      m_param_storeTrueHitsName = m_storeTrueHits.getName();
      m_storeDATCONSVDDigits.registerRelationTo(m_storeTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
    }

    m_storeMCParticles.isOptional(m_param_storeMCParticlesName);
    if (m_storeMCParticles.isValid()) {
      m_param_storeMCParticlesName = m_storeMCParticles.getName();
      m_storeDATCONSVDDigits.registerRelationTo(m_storeMCParticles, DataStore::c_Event, DataStore::c_DontWriteOut);
    }
  }
}

void SVDShaperDigitConverter::apply(std::vector<DATCONSVDDigit>& svdUDigits, std::vector<DATCONSVDDigit>& svdVDigits)
{
  for (auto& shaperdigit : m_storeSVDShaperDigits) {

    VxdID sensorid = shaperdigit.getSensorID();
    bool isu = shaperdigit.isUStrip();
    short cellid = shaperdigit.getCellID();
    DATCONSVDDigit::APVFloatSamples rawsamples = shaperdigit.getSamples();

    if (isu) {
      svdUDigits.emplace_back(DATCONSVDDigit(sensorid, isu, cellid, rawsamples));
    } else {
      svdVDigits.emplace_back(DATCONSVDDigit(sensorid, isu, cellid, rawsamples));
    }

    if (m_param_saveStoreArrays) {
      DATCONSVDDigit newDATCONSVDDigit(sensorid, isu, cellid, rawsamples);
      DATCONSVDDigit* datcondigit = m_storeDATCONSVDDigits.appendNew(newDATCONSVDDigit);

      // Register relation to the SVDShaperDigit the DATCONSVDDigit is derived from
      datcondigit->addRelationTo(&shaperdigit);
      RelationVector<MCParticle> relatedMC = shaperdigit.getRelationsTo<MCParticle>();
      RelationVector<SVDTrueHit> relatedSVDTrue = shaperdigit.getRelationsTo<SVDTrueHit>();
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

  }
}
