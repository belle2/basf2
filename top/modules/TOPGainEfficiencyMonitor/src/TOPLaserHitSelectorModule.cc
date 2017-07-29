/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPGainEfficiencyMonitor/TOPLaserHitSelectorModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>

#include <top/geometry/TOPGeometryPar.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <set>
#include <map>
#include <TMath.h>
#include <TF1.h>

using namespace std;
using namespace Belle2;

REG_MODULE(TOPLaserHitSelector)


TOPLaserHitSelectorModule::TOPLaserHitSelectorModule() : HistoModule()
{
  setDescription("TOP pixel-by-pixel gain analysis - first step : create hit timing-pulse height histogram");

  m_timeHistogramBinning.push_back(140);
  m_timeHistogramBinning.push_back(-25);
  m_timeHistogramBinning.push_back(45);
  m_heightHistogramBinning.push_back(150);
  m_heightHistogramBinning.push_back(-50);
  m_heightHistogramBinning.push_back(1450);

  addParam("timeHistogramBinning", m_timeHistogramBinning,
           "histogram binning (the number of bins, lower limit, upper limit) for hit timing distribution (should be integer value)",
           m_timeHistogramBinning);
  addParam("heightHistogramBinning", m_heightHistogramBinning,
           "histogram binning (the number of bins, lower limit, upper limit) for pulse height distribution (should be integer value)",
           m_heightHistogramBinning);

  //  addParam("calibrationChannel", m_calibrationChannel, "asic channel # where calibration pulses routed",
  //           (unsigned)0);
  addParam("useDoublePulse", m_useDoublePulse,
           "set true when you require both of double calibration pulses for reference timing. You need to enable offline feature extraction.",
           (bool)true);
  addParam("minHeightFirstCalPulse", m_calibrationPulseThreshold1, "pulse height threshold for the first cal. pulse",
           (float)600.);
  addParam("minHeightSecondCalPulse", m_calibrationPulseThreshold1, "pulse height threshold for the second cal. pulse",
           (float)450.);
  addParam("nominalDeltaT", m_calibrationPulseInterval, "nominal DeltaT (time interval of the double calibration pulses) [ns]",
           (float)21.85);
  addParam("nominalDeltaTRange", m_calibrationPulseIntervalRange,
           "acceptable DeltaT range from the nominal value before calibration [ns]",
           (float)2.);

}

TOPLaserHitSelectorModule::~TOPLaserHitSelectorModule() {}

void TOPLaserHitSelectorModule::initialize()
{
  REG_HISTOGRAM;

  //StoreArray<TOPRawDigit> rawDigits;
  //rawDigits.isRequired();
  StoreArray<TOPDigit> digits;
  digits.isRequired();
}

void TOPLaserHitSelectorModule::defineHisto()
{
  for (int iPixel = 0 ; iPixel < c_NPixelPerModule * c_NModule ; iPixel++) {

    short slotId = (iPixel / c_NPixelPerModule) + 1;
    short pixelId = (iPixel % c_NPixelPerModule) + 1;
    short pmtId = ((pixelId - 1) % c_NPixelPerRow) / c_NChannelPerPMTRow + c_NPMTPerRow * ((pixelId - 1) / (c_NPixelPerModule / 2)) + 1;
    short pmtChId = (pixelId - 1) % c_NChannelPerPMTRow + c_NChannelPerPMTRow * ((pixelId - 1) %
                    (c_NPixelPerModule / 2) / c_NPixelPerRow) + 1;

    std::ostringstream pixelstr;
    pixelstr << "s" << std::setw(2) << std::setfill('0') << slotId << "_PMT"
             << std::setw(2) << std::setfill('0') << pmtId
             << "_" << std::setw(2) << std::setfill('0') << pmtChId;
    std::ostringstream hname;
    hname << "hTimeHeight_" << pixelstr.str();
    std::ostringstream htitle;
    htitle << "2D distribution of hit timing and pulse height for " << pixelstr.str();
    m_timeHeightHistogram[iPixel] = new TH2F(hname.str().c_str(), htitle.str().c_str(),
                                             m_timeHistogramBinning[0], m_timeHistogramBinning[1], m_timeHistogramBinning[2],
                                             m_heightHistogramBinning[0], m_heightHistogramBinning[1], m_heightHistogramBinning[2]);
  }
}

void TOPLaserHitSelectorModule::beginRun()
{
}

void TOPLaserHitSelectorModule::event()
{
  StoreArray<TOPDigit> digits;

  std::map<short, float> refTimingMap;//map of pixel id and time
  std::map<short, std::vector<hitInfo_t> >
  calPulsesMap;//map of pixel id and hit information (timing and pulse height) for cal. pulse candidetes

  //first, identify cal. pulse
  for (const auto& digit : digits) {

    short slotId = digit.getModuleID();
    short pixelId = digit.getPixelID();
    short globalPixelId = (slotId - 1) * c_NPixelPerModule + pixelId - 1;
    if (digit.getHitQuality() != TOPDigit::c_CalPulse) continue;

    calPulsesMap[globalPixelId].push_back((hitInfo_t) { (float)digit.getTime(), (float)digit.getPulseHeight() });
  }

  //cal. pulse timing calculation
  for (const auto& calPulse : calPulsesMap) {

    short globalPixelId = calPulse.first;
    short globalAsicId = globalPixelId / c_NChannelPerAsic;
    std::vector<hitInfo_t> vec = calPulse.second;
    if (!m_useDoublePulse) {
      if (vec.size() == 1) {
        if (vec[0].m_height > m_calibrationPulseThreshold1)
          refTimingMap[globalAsicId] = vec[0].m_time;
      }
    } else {
      unsigned nCalPulseCand = vec.size();
      for (unsigned iVec = 0 ; iVec < nCalPulseCand ; iVec++) {
        for (unsigned jVec = 0 ; jVec < nCalPulseCand ; jVec++) {

          if (iVec == jVec || vec[iVec].m_time > vec[jVec].m_time) continue;
          if (vec[iVec].m_height > m_calibrationPulseThreshold1
              && vec[iVec].m_height > m_calibrationPulseThreshold2
              && TMath::Abs(vec[jVec].m_time - vec[iVec].m_time - m_calibrationPulseInterval) < m_calibrationPulseIntervalRange) {

            //in case multiple candidates of double cal. pulses are found,
            //choose a pair with the earliest 1st cal. pulse timing
            if (refTimingMap.count(globalAsicId) == 0 || refTimingMap[globalAsicId] > vec[iVec].m_time)
              refTimingMap[globalAsicId] = vec[iVec].m_time;
          }
        }//for(jVec)
      }//for(iVec)
    }//if(m_useDoublePulse)
  }//for(pair)


  //calculate hit atiming with respect to cal. pulse and fill hit info. histogram
  for (const auto& digit : digits) {

    short slotId = digit.getModuleID();
    short pixelId = digit.getPixelID();
    short globalPixelId = (slotId - 1) * c_NPixelPerModule + pixelId - 1;
    short globalAsicId = globalPixelId / c_NChannelPerAsic;
    if (digit.getHitQuality() == TOPDigit::c_Junk
        || digit.getHitQuality() == TOPDigit::c_CalPulse) continue;

    float hitTime = digit.getTime() - refTimingMap[globalAsicId];
    float pulseHeight = digit.getPulseHeight();

    m_timeHeightHistogram[globalPixelId]->Fill(hitTime, pulseHeight);
  }

}

void TOPLaserHitSelectorModule::endRun()
{
}

void TOPLaserHitSelectorModule::terminate()
{
}
