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

#include <iostream>
#include <vector>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDShaperDigitsGrouping);

SVDShaperDigitsGroupingModule::SVDShaperDigitsGroupingModule() :
  Module()
{
  setDescription("Calculate CoG3 time of each strip and grouping them.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name.", string(""));
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

    double stripTime = 0;
    float stripSumAmplitudes = 0;

    //take the MaxSum 3 samples
    SVD::SVDMaxSumAlgorithm maxSum = SVD::SVDMaxSumAlgorithm(currentDigit.getSamples());
    std::vector<float> selectedSamples = maxSum.getSelectedSamples();

    auto begin = selectedSamples.begin();
    const auto end = selectedSamples.end();

    for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
      stripSumAmplitudes += static_cast<double>(*begin);
      stripTime += static_cast<double>(*begin) * step;
    }
    if (stripSumAmplitudes != 0) {
      stripTime /= (stripSumAmplitudes);
    } else {
      stripTime = -1;
      B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
    }

    if (isnan(m_triggerBin))
      B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

    //cellID = 10 not used for calibration
    stripTime = m_CoG3TimeCal.getCorrectedTime(currentDigit.getSensorID(), currentDigit.isUStrip(), 10, stripTime, m_triggerBin);

    currentDigit.setTime(stripTime);
    // std::cout<<" "<<currentDigit.getTime();

  } // for (SVDShaperDigit& currentDigit : m_storeDigits) {
  // std::cout<<"\n";

}

