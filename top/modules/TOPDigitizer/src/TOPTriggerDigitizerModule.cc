/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Own include
#include <top/modules/TOPDigitizer/TOPTriggerDigitizerModule.h>

// Hit classes
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPTriggerDigit.h>
#include <top/dataobjects/TOPTriggerMCInfo.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPTriggerDigitizer)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPTriggerDigitizerModule::TOPTriggerDigitizerModule() : Module()
  {
    // Set description()
    setDescription("Digitizer that provides time stamps for TOP trigger");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("threshold", m_threshold,
             "pulse height threshold [ADC counts]", 27);
    addParam("hysteresis", m_hysteresis,
             "pulse height threshold hysteresis [ADC counts]", 10);
    addParam("gateWidth", m_gateWidth,
             "width of discriminator gate [samples]", 8);
    addParam("samplingPhase", m_samplingPhase,
             "sampling phase [samples]", 7);
  }

  TOPTriggerDigitizerModule::~TOPTriggerDigitizerModule()
  {
  }

  void TOPTriggerDigitizerModule::initialize()
  {
    // input
    StoreArray<TOPRawWaveform> waveforms;
    waveforms.isRequired();

    // output
    StoreArray<TOPTriggerDigit> digits;
    digits.registerInDataStore();
    digits.registerRelationTo(waveforms);
    StoreObjPtr<TOPTriggerMCInfo> mcInfo;
    mcInfo.registerInDataStore();

    if (m_samplingPhase < 0 or m_samplingPhase >= c_SamplingCycle)
      B2ERROR("samplingPhase must be positive and less than " << c_SamplingCycle);

  }

  void TOPTriggerDigitizerModule::beginRun()
  {
  }

  void TOPTriggerDigitizerModule::event()
  {

    // input: simulated waveforms
    StoreArray<TOPRawWaveform> waveforms;

    // output: time stamps for trigger input
    StoreArray<TOPTriggerDigit> digits;
    StoreObjPtr<TOPTriggerMCInfo> mcInfo;
    mcInfo.create();

    if (waveforms.getEntries() == 0) {
      B2ERROR("No waveforms available for digitization");
      return;
    }
    unsigned revo9count = waveforms[0]->getRevo9Counter();
    int offsetSamples = waveforms[0]->getOffsetWindows() * TOPRawWaveform::c_WindowSize +
                        (revo9count % 6) * TOPRawWaveform::c_WindowSize / 3;

    int bunchTimeStamp = int((revo9count + gRandom->Rndm()) * c_SamplingCycle / 3.0);
    mcInfo->setBunchTimeStamp(bunchTimeStamp);

    int offset = bunchTimeStamp - offsetSamples / c_SamplingCycle;

    for (const auto& waveform : waveforms) {
      const auto& data = waveform.getWaveform();
      int currentThr = m_threshold;
      bool lastState = false;
      int gate = 0;
      TOPTriggerDigit* digit = 0;
      for (int i = 0; i < (int) data.size(); i++) {
        gate--;
        if (data[i] > currentThr) {
          currentThr = m_threshold - m_hysteresis;
          if (!lastState) gate = m_gateWidth;
          lastState = true;
        } else {
          currentThr = m_threshold;
          lastState =  false;
        }
        if (i % c_SamplingCycle == m_samplingPhase and gate > 0) {
          if (i / c_SamplingCycle == 0) continue; // first time stamp is not correct
          if (!digit) {
            digit = digits.appendNew(waveform.getModuleID(),
                                     waveform.getChannel(),
                                     waveform.getScrodID());
            digit->addRelationTo(&waveform);
          }
          int timeStamp = i / c_SamplingCycle + offset;
          while (timeStamp < 0) {timeStamp += c_Frame9Period;};
          timeStamp = timeStamp % c_Frame9Period;
          digit->appendTimeStamp(timeStamp);
        }
      }
    }

  }


  void TOPTriggerDigitizerModule::endRun()
  {

  }

  void TOPTriggerDigitizerModule::terminate()
  {

  }


} // end Belle2 namespace

