/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPWaveformFeatureExtractor/TOPWaveformFeatureExtractorModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>


// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPWaveformSegment.h>
#include <framework/dataobjects/EventMetaData.h>

#include <top/geometry/TOPGeometryPar.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPWaveformFeatureExtractor)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPWaveformFeatureExtractorModule::TOPWaveformFeatureExtractorModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Module adds raw digits that are found in waveforms "
                   "but not already present in TOPRawDigits. "
                   "Only waveforms related to TOPRawDigits are used.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("inputRawDigitsName", m_inputRawDigitsName,
             "name of TOPRawDigit store array", string(""));
    addParam("threshold", m_threshold,
             "pulse height threshold [ADC counts]", 40);
    addParam("hysteresis", m_hysteresis,
             "threshold hysteresis [ADC counts]", 10);
    addParam("thresholdCount", m_thresholdCount,
             "minimal number of samples above threshold", 3);
    addParam("setIntegral", m_setIntegral,
             "calculate and set integral for online-extracted hits", true);

  }

  TOPWaveformFeatureExtractorModule::~TOPWaveformFeatureExtractorModule()
  {
  }

  void TOPWaveformFeatureExtractorModule::initialize()
  {

    StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
    rawDigits.isRequired();

  }

  void TOPWaveformFeatureExtractorModule::beginRun()
  {
  }


  void TOPWaveformFeatureExtractorModule::event()
  {

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    const auto& tdc = geo->getNominalTDC();
    int sampleDivisions = 0x1 << tdc.getSubBits();

    StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
    int initSize = rawDigits.getEntries();

    for (int i = 0; i < initSize; i++) {
      auto& rawDigit = *rawDigits[i];
      auto* rawWaveform = rawDigit.getRelated<TOPRawWaveform>();
      auto* waveformSegment = rawDigit.getRelated<TOPWaveformSegment>();

      RelationsObject const* usedWaveform = 0;

      std::vector<TOP::FeatureExtractionData> extractedFeatures;

      if (rawWaveform) { //found a related TOPRawWaveform


        if (m_setIntegral) {
          auto integral = rawWaveform->getIntegral(rawDigit.getSampleRise(),
                                                   rawDigit.getSamplePeak(),
                                                   rawDigit.getSampleFall());
          rawDigit.setIntegral(integral);
        }
        rawWaveform->featureExtraction(m_threshold, m_hysteresis, m_thresholdCount);
        extractedFeatures = rawWaveform->getFeatureExtractionData();

        usedWaveform = rawWaveform;

      } else if (waveformSegment) { //found a related TOPWaveformSegment
        extractedFeatures = featureExtraction(waveformSegment, m_threshold, m_hysteresis, m_thresholdCount);

        usedWaveform = waveformSegment;

      } else {
        continue; //no related waveform found, continue
      }

      int sampleRise = rawDigit.getSampleRise();
      int sampleFall = rawDigit.getSampleFall() + 1;
      for (const auto& feature : extractedFeatures) {

        // skip it, if hit already in rawDigits
        int sRise = feature.sampleRise;
        if (sRise >= sampleRise and sRise <= sampleFall) continue;
        int sFall = feature.sampleFall + 1;
        if (sFall >= sampleRise and sFall <= sampleFall) continue;

        // if not, append it
        auto* newDigit = rawDigits.appendNew(rawDigit);
        newDigit->setOfflineFlag();
        newDigit->setSampleRise(feature.sampleRise);
        newDigit->setDeltaSamplePeak(feature.samplePeak - feature.sampleRise);
        newDigit->setDeltaSampleFall(feature.sampleFall - feature.sampleRise);
        newDigit->setValueRise0(feature.vRise0);
        newDigit->setValueRise1(feature.vRise1);
        newDigit->setValueFall0(feature.vFall0);
        newDigit->setValueFall1(feature.vFall1);
        newDigit->setValuePeak(feature.vPeak);
        newDigit->setIntegral(feature.integral);
        double rawTime = newDigit->getCFDLeadingTime();
        unsigned tfine = int(rawTime * sampleDivisions) % sampleDivisions; // TODO: <0 ?
        newDigit->setTFine(tfine);
        newDigit->addRelationTo(usedWaveform);
      }
    }

    int finalSize = rawDigits.getEntries();
    B2DEBUG(100, "TOPWaveformFeatureExtractor: appended " << finalSize - initSize
            << " raw digits to initial " << initSize);

  }


  void TOPWaveformFeatureExtractorModule::endRun()
  {
  }

  void TOPWaveformFeatureExtractorModule::terminate()
  {
  }

  int TOPWaveformFeatureExtractorModule::getIntegral(TOPWaveformSegment* waveformSegment, int sampleRise, int samplePeak,
                                                     int sampleFall)
  {
    auto& waveform = waveformSegment->getWaveform();

    int min = samplePeak - 3 * (samplePeak - sampleRise);
    if (min < 0) min = 0;
    int max = samplePeak + 4 * (sampleFall + 1 - samplePeak);
    int size = waveform.size();
    if (max > size) max = size;
    int integral = 0;
    while (min < max) {
      integral += waveform[min];
      min++;
    }
    return integral;
  }

  std::vector<TOP::FeatureExtractionData> TOPWaveformFeatureExtractorModule::featureExtraction(TOPWaveformSegment* waveformSegment,
      int threshold, int hysteresis,
      int thresholdCount)
  {

    std::vector<TOP::FeatureExtractionData> features;
    auto& waveform = waveformSegment->getWaveform();
    auto samples = waveformSegment->getSampleNumbers();

    int currentThr = threshold;
    bool lastState = false;
    int samplePeak = 0;
    int size = waveform.size();
    int aboveThr = 0;
    for (int i = 0; i < size; i++) {
      const auto& adc = waveform[i];
      if (adc > currentThr and i < size - 1) {
        currentThr = threshold - hysteresis;
        if (!lastState or adc > waveform[samplePeak]) samplePeak = i;
        lastState = true;
        aboveThr++;
      } else {
        currentThr = threshold;
        if (lastState and aboveThr >= thresholdCount) {
          auto halfValue = waveform[samplePeak] / 2;
          auto sampleRise = samplePeak;
          while (sampleRise > 0 and waveform[sampleRise] > halfValue) sampleRise--;
          if (sampleRise == 0 and waveform[sampleRise] > halfValue) continue;
          auto sampleFall = samplePeak;
          while (sampleFall < size - 1 and waveform[sampleFall] > halfValue) sampleFall++;
          if (sampleFall == size - 1 and waveform[sampleFall] > halfValue) continue;
          sampleFall--;
          TOP::FeatureExtractionData feature;
          feature.sampleRise = samples[sampleRise];
          feature.samplePeak = samples[samplePeak];
          feature.sampleFall = samples[sampleFall];
          feature.vRise0 = waveform[sampleRise];
          feature.vRise1 = waveform[sampleRise + 1];
          feature.vPeak = waveform[samplePeak];
          feature.vFall0 = waveform[sampleFall];
          feature.vFall1 = waveform[sampleFall + 1];
          feature.integral = getIntegral(waveformSegment, sampleRise, samplePeak, sampleFall);
          features.push_back(feature);
        }
        lastState = false;
        aboveThr = 0;
      }
    }

    return features;
  }


} // end Belle2 namespace

