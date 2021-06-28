/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/SVDShaperDigitConverter.h>
#include <tracking/datcon/fpgaDATCON/entities/DATCONSVDDigit.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SVDShaperDigitConverter::SVDShaperDigitConverter() : Super()
{
}

void SVDShaperDigitConverter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeSVDShaperDigitsName"), m_param_storeSVDShaperDigitsName,
                                "StoreArray name of the SVDShaperDigits StoreArray.", m_param_storeSVDShaperDigitsName);
}

void SVDShaperDigitConverter::initialize()
{
  Super::initialize();

  m_storeSVDShaperDigits.isRequired(m_param_storeSVDShaperDigitsName);
}

void SVDShaperDigitConverter::apply(std::vector<DATCONSVDDigit>& svdUDigits, std::vector<DATCONSVDDigit>& svdVDigits)
{
  for (auto& shaperdigit : m_storeSVDShaperDigits) {

    const VxdID& sensorid = shaperdigit.getSensorID();
    bool isu = shaperdigit.isUStrip();
    short cellid = shaperdigit.getCellID();
    const DATCONSVDDigit::APVFloatSamples& rawsamples = shaperdigit.getSamples();

    if (isu) {
      svdUDigits.emplace_back(DATCONSVDDigit(sensorid, isu, cellid, rawsamples));
    } else {
      svdVDigits.emplace_back(DATCONSVDDigit(sensorid, isu, cellid, rawsamples));
    }
  }
}
