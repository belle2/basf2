/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdShaperDigitsSlope/SVDShaperDigitsSlopeModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <svd/dataobjects/SVDEventInfo.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDShaperDigitsSlope);

SVDShaperDigitsSlopeModule::SVDShaperDigitsSlopeModule() :
  Module(),
  m_cutAdjacent(3.0), m_useDB(true)
{
  setDescription("Calculate slope of shaper digits.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("EventInfo", m_svdEventInfoName,
           "SVDEventInfo collection name.", string("SVDEventInfo"));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name.", string(""));

  addParam("useDB", m_useDB,
           "if False, use clustering and reconstruction configuration module parameters", m_useDB);
  addParam("AdjacentSN", m_cutAdjacent,
           "minimum SNR for strips to be considered for clustering. Overwritten by the dbobject, unless you set useDB = False.",
           m_cutAdjacent);
}



void SVDShaperDigitsSlopeModule::initialize()
{
  m_storeDigits.isRequired(m_storeShaperDigitsName);

  B2DEBUG(1, "SVDShaperDigitsSlopeModule \nsvdShaperDigits: " << m_storeDigits.getName());
}



void SVDShaperDigitsSlopeModule::event()
{
  int nDigits = m_storeDigits.getEntries();
  if (nDigits == 0)
    return;

  //loop over the SVDShaperDigits
  for (SVDShaperDigit& currentDigit : m_storeDigits)
    calculateSlope(currentDigit);

}



void SVDShaperDigitsSlopeModule::calculateSlope(SVDShaperDigit& currentDigit)
{

  if (m_useDB)
    m_cutAdjacent = m_ClusterCal.getMinAdjSNR(currentDigit.getSensorID(), currentDigit.isUStrip());

  //Ignore digits with insufficient signal
  float thisNoise = m_NoiseCal.getNoise(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID());
  int thisCharge = currentDigit.getMaxADCCounts();
  B2DEBUG(20, "Noise = " << thisNoise << " ADC, MaxSample = " << thisCharge << " ADC");

  float slope = - TMath::Pi() * 0.5;
  if ((float)thisCharge / thisNoise < m_cutAdjacent) {
    currentDigit.setSlope(slope);
    return;
  }

  float szxy = 0;
  float   sz = 0;
  float  sxy = 0;
  float  sz2 = 0;
  float   sn = 0;

  // float intersect = -10000;

  Belle2::SVDShaperDigit::APVFloatSamples samples = currentDigit.getSamples();

  // slope w.r.t. Z-axis.
  for (int ij = 0; ij < static_cast<int>(samples.size()); ij ++) {
    float xzval[2] = {samples.at(ij), float(ij + 1)};
    szxy += xzval[1] * xzval[0];
    sz   += xzval[1];
    sz2  += xzval[1] * xzval[1];
    sxy  += xzval[0];
    sn   += 1.;
  } // for (int ij = 0; ij < static_cast<int>(samples.size()); ij ++) {

  if (sn > 0. && sz2 * sn - sz * sz != 0.) {
    slope = (szxy * sn - sz * sxy) / (sz2 * sn - sz * sz);
    // intersect = sxy/sn - tmpslp*sz/sn;
  }

  currentDigit.setSlope(TMath::ATan(slope));

}
