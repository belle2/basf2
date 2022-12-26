/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdShaperDigitsGrouping/SVDShaperDigitsGroupingModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <svd/dataobjects/SVDEventInfo.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDShaperDigitsGrouping);

SVDShaperDigitsGroupingModule::SVDShaperDigitsGroupingModule() :
  Module(),
  m_cutAdjacent(3.0), m_useDB(true)
{
  setDescription("Calculate CoG3 time of each strip and grouping them.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("EventInfo", m_svdEventInfoName,
           "SVDEventInfo collection name.", string("SVDEventInfo"));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name.", string(""));

  // 2. Clustering
  addParam("useDB", m_useDB,
           "if False, use clustering and reconstruction configuration module parameters", m_useDB);
  addParam("AdjacentSN", m_cutAdjacent,
           "minimum SNR for strips to be considered for clustering. Overwritten by the dbobject, unless you set useDB = False.",
           m_cutAdjacent);
}



void SVDShaperDigitsGroupingModule::initialize()
{
  m_storeDigits.isRequired(m_storeShaperDigitsName);
}


void SVDShaperDigitsGroupingModule::beginRun()
{
  //read sampling time from HardwareClockSettings
  if (m_hwClock.isValid())
    m_apvClockPeriod = 1. / m_hwClock->getClockFrequency(Const::EDetector::SVD, "sampling");

  if (std::isnan(m_apvClockPeriod))
    B2FATAL("Forbidden state!!");
}

void SVDShaperDigitsGroupingModule::event()
{
  int nDigits = m_storeDigits.getEntries();
  if (nDigits == 0)
    return;

  //loop over the SVDShaperDigits
  // std::cout<<" strip times ";
  for (SVDShaperDigit& currentDigit : m_storeDigits) {

    if (m_useDB)
      m_cutAdjacent = m_ClusterCal.getMinAdjSNR(currentDigit.getSensorID(), currentDigit.isUStrip());

    //Ignore digits with insufficient signal
    float thisNoise = m_NoiseCal.getNoise(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID());
    int thisCharge = currentDigit.getMaxADCCounts();
    B2DEBUG(20, "Noise = " << thisNoise << " ADC, MaxSample = " << thisCharge << " ADC");

    if ((float)thisCharge / thisNoise < m_cutAdjacent)
      continue;

    float stripTime = 0;
    float stripSumAmplitudes = 0;

    Belle2::SVDShaperDigit::APVFloatSamples samples = currentDigit.getSamples();

#define CoG3Alg
#ifdef CoG6Alg

    int firstFrame = 0;
    for (int k = 0; k < static_cast<int>(samples.size()); k ++) {
      stripTime += k * samples.at(k);
      stripSumAmplitudes += samples.at(k);
    }

#endif
#ifdef CoG3Alg

    //take the MaxSum 3 samples
    SVD::SVDMaxSumAlgorithm maxSum = SVD::SVDMaxSumAlgorithm(samples);
    std::vector<float> selectedSamples = maxSum.getSelectedSamples();
    int firstFrame = maxSum.getFirstFrame();

    auto begin = selectedSamples.begin();
    const auto end = selectedSamples.end();

    for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
      stripSumAmplitudes += static_cast<float>(*begin);
      stripTime += static_cast<float>(*begin) * step;
    }
#endif

    if (stripSumAmplitudes != 0) {
      stripTime /= (stripSumAmplitudes);
    } else {
      stripTime = -1;
      B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
    }

    if (isnan(m_triggerBin))
      B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

#ifdef CoG6Alg

    stripTime -= m_PulseShapeCal.getPeakTime(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID());
    stripTime =  m_CoG6TimeCal.getCorrectedTime(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID(),
                                                stripTime, m_triggerBin);

#endif
#ifdef CoG3Alg

    //cellID = 10 not used for calibration
    stripTime = m_CoG3TimeCal.getCorrectedTime(currentDigit.getSensorID(), currentDigit.isUStrip(), 10, stripTime, m_triggerBin);

#endif

    //first take Event Informations:
    StoreObjPtr<SVDEventInfo> temp_eventinfo(m_svdEventInfoName);
    if (!temp_eventinfo.isValid())
      m_svdEventInfoName = "SVDEventInfoSim";
    StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
    if (!eventinfo) B2ERROR("No SVDEventInfo!");

    // now go into FTSW time reference frame
    stripTime = eventinfo->getTimeInFTSWReference(stripTime, firstFrame);

    currentDigit.setTime(stripTime);
    // std::cout<<" "<<currentDigit.getTime();

  } // for (SVDShaperDigit& currentDigit : m_storeDigits) {
  // std::cout<<"\n";

}

