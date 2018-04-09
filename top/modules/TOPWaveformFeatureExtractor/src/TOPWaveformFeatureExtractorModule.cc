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

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
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
             "pulse height threshold [ADC counts]", 10);
    addParam("hysteresis", m_hysteresis,
             "threshold hysteresis [ADC counts]", 3);
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
      const auto* waveform = rawDigit.getRelated<TOPRawWaveform>();
      if (!waveform) continue;
      if (m_setIntegral) {
        auto integral = waveform->getIntegral(rawDigit.getSampleRise(),
                                              rawDigit.getSamplePeak(),
                                              rawDigit.getSampleFall());
        rawDigit.setIntegral(integral);
      }
      waveform->featureExtraction(m_threshold, m_hysteresis, m_thresholdCount);
      const auto& features = waveform->getFeatureExtractionData();
      int sampleRise = rawDigit.getSampleRise();
      int sampleFall = rawDigit.getSampleFall() + 1;
      for (const auto& feature : features) {

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
        newDigit->addRelationTo(waveform);
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


} // end Belle2 namespace

