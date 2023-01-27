/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/top/modules/trgtopTRD2TTSConverter/trgtopTRD2TTSConverterModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

#include <iostream>

//using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGTOPTRD2TTSConverter);

std::string TRGTOPTRD2TTSConverterModule::version() const
{
  return std::string("1.00");
}

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGTOPTRD2TTSConverterModule::TRGTOPTRD2TTSConverterModule()
  : Module::Module(), m_eventNumber(0)
{
  // Set module properties


  std::string desc = "TRGTOPTRD2TTSConverterModule(" + version() + ")" + "converts TOPRawDigits to TOP TRG timestamps";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);

  B2DEBUG(20, "TRGTOPTRD2TTSConverter: Constructor done.");

  // Parameter definitions

  // Add parameters
  addParam("inputRawDigitsName", m_inputRawDigitsName,
           "name of TOPRawDigit store array", std::string(""));

  addParam("inputConvertedDigitsName", m_inputConvertedDigitsName,
           "name of TOPDigit store array", std::string(""));

  // Add parameters
  addParam("outputTimeStampsSlotName", m_outputTimeStampsSlotName,
           "name of TRGTOPTimeStampsSlot store array", std::string(""));

  addParam("outputTimeStampName", m_outputTimeStampName,
           "name of TRGTOPTimeStamp store array", std::string(""));

  addParam("addRelations", m_addRelations, "if true, make relations to TOPRawDigits", true);

  addParam("requireMinNumberOfTimeStamps", m_requireMinNumberOfTimeStamps, "if true, require minimum number of timestamps in a slot",
           true);

  addParam("minNumberOfTimeStamps", m_minNumberOfTimeStamps, "minimum number of timestamps in a slot (when required, default:5)",
           MIN_NUMBER_OF_TIMESTAMPS);

}

TRGTOPTRD2TTSConverterModule::~TRGTOPTRD2TTSConverterModule()
{
}

void TRGTOPTRD2TTSConverterModule::initialize()
{
  m_convertedDigits.isRequired(m_inputConvertedDigitsName);
  m_rawDigits.isRequired(m_inputRawDigitsName);

  m_TRGTOPTimeStampsSlots.registerInDataStore(m_outputTimeStampsSlotName);
  m_TRGTOPTimeStamps.registerInDataStore(m_outputTimeStampName);

  m_TRGTOPTimeStampsSlots.registerRelationTo(m_TRGTOPTimeStamps);
  m_TRGTOPTimeStamps.registerRelationTo(m_TRGTOPTimeStampsSlots);

  m_TRGTOPTimeStamps.registerRelationTo(m_convertedDigits);
  m_convertedDigits.registerRelationTo(m_TRGTOPTimeStamps);
}


void TRGTOPTRD2TTSConverterModule::beginRun()
{
}

void TRGTOPTRD2TTSConverterModule::event()
{

  // clear TimeStamps - this should be done elsewhere automatically
  //  m_TRGTOPTimeStampsSlots.clear();
  //  m_TRGTOPTimeStamps.clear();

  for (int slot = 0; slot < NUMBER_OF_TOP_SLOTS; slot++) m_interimTimeStamps[slot].clear();

  if (m_convertedDigits.getEntries() && m_rawDigits.getEntries()) {

    for (const auto& convertedDigit : m_convertedDigits) {

      // note that here we use convention 0 through 15
      int slot = convertedDigit.getModuleID() - 1;

      auto relRawDigits = convertedDigit.getRelationsTo<TOPRawDigit>();

      if (relRawDigits.size()) {

        int revo9Counter = relRawDigits[0]->getRevo9Counter();

        // latency was calibrated on 01/16/22 by taking an average between revo9 of L1 and TOPTRG decision for events where TOP and ECL agree with each other
        int deltaRevo9 = revo9Counter - latencyL1;

        // 11520 = 1280*9
        int revo9CounterEvent = deltaRevo9 >= 0 ? deltaRevo9 : revo9CounterMax + deltaRevo9 ;

        int phase = 0;

        for (int i = 1; i < 9; i++) {
          if (revo9CounterEvent < timeOfWindows[i]) break;
          phase++;
        }

        auto window = relRawDigits[0]->getASICWindow();
        auto sample = relRawDigits[0]->getSampleRise();

        if (phase >= 0 && phase <= 8) {

          // we use 0.375, because our timestamps are in units of 2ns, so we pretend that FTSW clock is 127MHz
          // will have to divide by 2 soon

          double timeStamp_ns_d = ((numberOfWindows[phase] + window) * 64 + sample) * 0.375 + 0.5 + timeCorrection;
          int timeStamp_ns = timeStamp_ns_d;

          interimTimeStamp thisTimeStamp;
          thisTimeStamp.slot = slot + 1;
          // TG & VS: Feb. 8, 2022, the value in FW is in 2ns units
          thisTimeStamp.value = timeStamp_ns / 2;
          thisTimeStamp.refDigit = &convertedDigit;

          m_interimTimeStamps[slot].push_back(thisTimeStamp);

        }
      }
    }

    for (int slot = 0; slot < NUMBER_OF_TOP_SLOTS; slot++) {

      int numberOfTimeStamps = m_interimTimeStamps[slot].size();

      if ((m_requireMinNumberOfTimeStamps && numberOfTimeStamps >= m_minNumberOfTimeStamps) || (!m_requireMinNumberOfTimeStamps
          && numberOfTimeStamps)) {


        // store decision in event store
        auto* timeStampsSlotStore = m_TRGTOPTimeStampsSlots.appendNew(slot + 1, numberOfTimeStamps);

        //  int i = m_TRGTOPTimeStampsSlots.getEntries() - 1;
        //  m_TRGTOPTimeStampsSlots[i]->setSlotId(slot);
        //  m_TRGTOPTimeStampsSlots[i]->setNumberOfTimeStamps(numberOfTimeStamps);
        // or
        //  timeStampsSlotStore->setSlotId(slot);
        //  timeStampsSlotStore->setNumberOfTimeStamps(numberOfTimeStamps);

        // sort timestamps in increasing order of their values (i.e. time)

        sort(m_interimTimeStamps[slot].begin(), m_interimTimeStamps[slot].end(), timeOrder());

        for (std::vector<interimTimeStamp>::const_iterator it = m_interimTimeStamps[slot].begin(); it != m_interimTimeStamps[slot].end();
             ++it) {

          const interimTimeStamp& thisInterimTimeStamp = *it;

          int value = thisInterimTimeStamp.value;

          TRGTOPTimeStamp timeStamp(value, slot + 1);

          auto* timeStampStore = m_TRGTOPTimeStamps.appendNew(timeStamp);

          timeStampsSlotStore->addRelationTo(timeStampStore);
          timeStampStore->addRelationTo(timeStampsSlotStore);

          if (m_addRelations) {
            timeStampStore->addRelationTo(thisInterimTimeStamp.refDigit);
            thisInterimTimeStamp.refDigit->addRelationTo(timeStampStore);
          }
        }
      }
    }
  }
}

void TRGTOPTRD2TTSConverterModule::endRun()
{
}

void TRGTOPTRD2TTSConverterModule::terminate()
{
}



