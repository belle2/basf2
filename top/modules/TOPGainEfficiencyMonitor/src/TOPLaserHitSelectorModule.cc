/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke, Okuto Rikuya                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// own include
#include <top/modules/TOPGainEfficiencyMonitor/TOPLaserHitSelectorModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

// Hit classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>

#include <top/geometry/TOPGeometryPar.h>

// standard libraries
#include <iostream>
#include <sstream>
#include <iomanip>

#include <set>
#include <map>

// ROOT
#include <TMath.h>
#include <TF1.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPLaserHitSelector)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPLaserHitSelectorModule::TOPLaserHitSelectorModule() : HistoModule()
  {
    // Set description()
    setDescription("TOP pixel-by-pixel gain analysis - first step : create hit timing-pulse charge histogram");

    // Add parameters
    m_timeHistogramBinning.push_back(140);//140 for time axis
    m_timeHistogramBinning.push_back(-55);//-25
    m_timeHistogramBinning.push_back(15);//45
    m_chargeHistogramBinning.push_back(125);//for height
    m_chargeHistogramBinning.push_back(-50);//
    m_chargeHistogramBinning.push_back(2450);//
    m_chargeHistogramBinning.push_back(150);//for integral
    m_chargeHistogramBinning.push_back(-500);//
    m_chargeHistogramBinning.push_back(29500);//

    addParam("timeHistogramBinning", m_timeHistogramBinning,
             "histogram binning (the number of bins, lower limit, upper limit) for hit timing distribution (should be integer value)",
             m_timeHistogramBinning);
    addParam("chargeHistogramBinning", m_chargeHistogramBinning,
             "histogram binning (the number of bins, lower limit, upper limit) for pulse charge distribution (should be integer value)",
             m_chargeHistogramBinning);

    //  addParam("calibrationChannel", m_calibrationChannel, "asic channel # where calibration pulses routed",
    //           (unsigned)0);
    addParam("useDoublePulse", m_useDoublePulse,
             "set true when you require both of double calibration pulses for reference timing. You need to enable offline feature extraction.",
             (bool)true);
    addParam("minHeightFirstCalPulse", m_calibrationPulseThreshold1, "pulse height threshold for the first cal. pulse",
             (float)300.);
    addParam("minHeightSecondCalPulse", m_calibrationPulseThreshold2, "pulse height threshold for the second cal. pulse",
             (float)100.);
    addParam("nominalDeltaT", m_calibrationPulseInterval, "nominal DeltaT (time interval of the double calibration pulses) [ns]",
             (float)25.5);
    addParam("nominalDeltaTRange", m_calibrationPulseIntervalRange,
             "acceptable DeltaT range from the nominal value before calibration [ns]",
             (float)2.);
    addParam("windowSelect", m_windowSelect,
             "select window number (All=0, Odd=2, Even=1)",
             0);
    addParam("includePrimaryChargeShare", m_includePrimaryChargeShare,
             "set ture when you require without primary chargeshare cut for making 2D histogram",
             (bool)false);
    addParam("includeAllChargeShare", m_includeAllChargeShare,
             "set ture when you require without all chargeshare cut for making 2D histogram",
             (bool)false);
  }

  TOPLaserHitSelectorModule::~TOPLaserHitSelectorModule() {}

  void TOPLaserHitSelectorModule::initialize()
  {
    REG_HISTOGRAM;

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

      std::ostringstream hnameForgain;
      hnameForgain << "hTimeHeight_gain_" << pixelstr.str();
      std::ostringstream htitleForgain;
      htitleForgain << "2D distribution of hit timing and pulse height for Gain" << pixelstr.str();
      m_TimeHeightHistogramForFit[iPixel] = new TH2F(hnameForgain.str().c_str(), htitleForgain.str().c_str(),
                                                     m_timeHistogramBinning[0], m_timeHistogramBinning[1], m_timeHistogramBinning[2],
                                                     m_chargeHistogramBinning[0], m_chargeHistogramBinning[1],
                                                     m_chargeHistogramBinning[2]);

      std::ostringstream hnameForIntegral;
      hnameForIntegral << "hTimeIntegral_gain_" << pixelstr.str();
      std::ostringstream htitleForIntegral;
      htitleForIntegral << "2D distribution of hit timing and integral for Gain" << pixelstr.str();
      m_TimeIntegralHistogramForFit[iPixel] = new TH2F(hnameForIntegral.str().c_str(), htitleForIntegral.str().c_str(),
                                                       m_timeHistogramBinning[0], m_timeHistogramBinning[1], m_timeHistogramBinning[2],
                                                       m_chargeHistogramBinning[3], m_chargeHistogramBinning[4],
                                                       m_chargeHistogramBinning[5]);

      std::ostringstream hnameForeff;
      hnameForeff << "hTimeHeight_efficiency_" << pixelstr.str();
      std::ostringstream htitleForeff;
      htitleForeff << "2D distribution of hit timing and pulse height for efficiency" << pixelstr.str();
      m_TimeHeightHistogramForHitRate[iPixel] = new TH2F(hnameForeff.str().c_str(), htitleForeff.str().c_str(),
                                                         m_timeHistogramBinning[0], m_timeHistogramBinning[1], m_timeHistogramBinning[2],
                                                         m_chargeHistogramBinning[0], m_chargeHistogramBinning[1],
                                                         m_chargeHistogramBinning[2]);
    }

    const short nAsic = c_NPixelPerModule / c_NChannelPerAsic * c_NChannelPerPMT;
    m_nCalPulseHistogram = new TH1F("hNCalPulse", "number of calibration pulses identificed for each asic",
                                    nAsic, -0.5, nAsic - 0.5);
  }

  void TOPLaserHitSelectorModule::beginRun()
  {
  }

  int counttest;
  void TOPLaserHitSelectorModule::event()
  {
    StoreArray<TOPDigit> digits;

    std::map<short, float> refTimingMap;//map of pixel id and time
    std::map<short, std::vector<hitInfo_t> >
    calPulsesMap;//map of pixel id and hit information (timing and pulse charge) for cal. pulse candidetes

    //first, identify cal. pulse
    for (const auto& digit : digits) {

      short slotId = digit.getModuleID();
      short pixelId = digit.getPixelID();
      short globalPixelId = (slotId - 1) * c_NPixelPerModule + pixelId - 1;
      if (digit.getHitQuality() == TOPDigit::c_CalPulse) {
        calPulsesMap[globalPixelId].push_back((hitInfo_t) { (float)digit.getTime(), (float)digit.getPulseHeight() });
      }
    }// for digit pair


    //cal. pulse timing calculation
    for (const auto& calPulse : calPulsesMap) {

      short globalPixelId = calPulse.first;
      short globalAsicId = globalPixelId / c_NChannelPerAsic;
      std::vector<hitInfo_t> vec = calPulse.second;
      double calPulseTiming = 9999999;
      unsigned nCalPulseCandidates = vec.size();
      if (!m_useDoublePulse) { //try to find the first cal. pulse in case that both of double cal. pulses are not required (choose the largest hit)
        double maxPulseHeight = -1;
        double maxHeightTiming = 9999999;
        for (unsigned iVec = 0 ; iVec < nCalPulseCandidates ; iVec++) {
          if (maxPulseHeight < vec[iVec].m_height) {
            maxPulseHeight = vec[iVec].m_height;
            maxHeightTiming = vec[iVec].m_time;
          }
        }
        if (maxPulseHeight > m_calibrationPulseThreshold1)
          calPulseTiming = maxHeightTiming;
      } else { //try to identify both of double cal. pulses when specified by option (default)
        for (unsigned iVec = 0 ; iVec < nCalPulseCandidates ; iVec++) {
          for (unsigned jVec = 0 ; jVec < nCalPulseCandidates ; jVec++) {

            if (iVec == jVec || vec[iVec].m_time > vec[jVec].m_time) continue;
            if (vec[iVec].m_height > m_calibrationPulseThreshold1
                && vec[iVec].m_height > m_calibrationPulseThreshold2
                && TMath::Abs(vec[jVec].m_time - vec[iVec].m_time - m_calibrationPulseInterval) < m_calibrationPulseIntervalRange) {

              //in case multiple candidates of double cal. pulses are found,
              //choose a pair with the earliest 1st cal. pulse timing
              if (refTimingMap.count(globalAsicId) == 0 || refTimingMap[globalAsicId] > vec[iVec].m_time)
                calPulseTiming = vec[0].m_time;
            }
          }//for(jVec)
        }//for(iVec)
      }//if(m_useDoublePulse)

      if (calPulseTiming < 9999998) { //when cal. pulse(s) are properly identified
        refTimingMap[globalAsicId] = calPulseTiming;
        m_nCalPulseHistogram->Fill(globalAsicId);
      }
    }//for(pair)

    //calculate hit timing with respect to cal. pulse and fill hit info. histogram
    for (const auto& digit : digits) {
      counttest++;
      short slotId = digit.getModuleID();
      short pixelId = digit.getPixelID();
      short globalPixelId = (slotId - 1) * c_NPixelPerModule + pixelId - 1;
      short globalAsicId = globalPixelId / c_NChannelPerAsic;
      if (counttest < 10)
        cout << digit.getTime() << " " << refTimingMap[globalAsicId] << " " << digit.getPulseHeight() << endl;

      if (digit.getHitQuality() == TOPDigit::c_Junk
          || digit.getHitQuality() == TOPDigit::c_CrossTalk) continue;

      float hitTime = digit.getTime() - refTimingMap[globalAsicId];
      float pulseHeight = digit.getPulseHeight();
      float Integral = digit.getIntegral();
      short windowNumberOfHit = (short)(digit.getFirstWindow()) + (short)(digit.getRawTime() / 64);
      if (m_windowSelect && windowNumberOfHit % 2 != (m_windowSelect - 1)) continue;

      if (m_includeAllChargeShare) {
        m_TimeHeightHistogramForHitRate[globalPixelId]->Fill(hitTime, pulseHeight);
        m_TimeIntegralHistogramForFit[globalPixelId]->Fill(hitTime, Integral);
        m_TimeHeightHistogramForFit[globalPixelId]->Fill(hitTime, pulseHeight);
        continue;
      }

      if (!digit.isSecondaryChargeShare()) {
        m_TimeHeightHistogramForHitRate[globalPixelId]->Fill(hitTime, pulseHeight);
        if (m_includePrimaryChargeShare) {
          m_TimeIntegralHistogramForFit[globalPixelId]->Fill(hitTime, Integral);
          m_TimeHeightHistogramForFit[globalPixelId]->Fill(hitTime, pulseHeight);
          continue;
        }
      }

      if (digit.isSecondaryChargeShare() || digit.isPrimaryChargeShare()) continue;
      m_TimeIntegralHistogramForFit[globalPixelId]->Fill(hitTime, Integral);
      m_TimeHeightHistogramForFit[globalPixelId]->Fill(hitTime, pulseHeight);
    }

  }

  void TOPLaserHitSelectorModule::endRun()
  {
  }

  void TOPLaserHitSelectorModule::terminate()
  {
  }

} // end Belle2 namespace
