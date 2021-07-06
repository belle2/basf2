/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDEventInfoSetterModule.h>
#include <root/TRandom.h>
#include <mdst/dataobjects/TRGSummary.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventInfoSetter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventInfoSetterModule::SVDEventInfoSetterModule() : Module()
{
  //Set module properties
  setDescription(
    "Sets the SVD event information. Use this "
    "module to fill SVDEventInfo object which tells the digitizer "
    "the conditions for creating ShaperDigits."
  );

  setPropertyFlags(c_ParallelProcessingCertified);

  //Parameter definition for SVDModeByte, TriggerType and cross-talk
  addParam("SVDEventInfo", m_svdEventInfoName, "SVDEventInfo name", string("SVDEventInfoSim"));
  addParam("runType", m_runType, "Defines the run type: raw/transparent/zero-suppressed/z-s+hit time finding", int(2));
  addParam("eventType", m_eventType, "Defines the event type: TTD event (global run)/standalone event (local run)", int(0));
  addParam("daqMode", m_daqMode,
           "Defines the DAQ mode:  = 2 for the default 6-sample mode, = 1 for the 3-sample mode, = 3 for the 3-mixed-6 sample mode. ", int(2));
  addParam("fixedTriggerBin", m_fixedTriggerBin,
           "Trigger bin 0/1/2/3 - useful for timing studies. The default is random if SimClockState is not valid.", int(999));
  addParam("triggerType", m_triggerType, "Defines the trigger type, default: CDC trigger", uint8_t(3));
  addParam("crossTalk", m_xTalk, "Defines the cross-talk flag for the event", bool(false));
  addParam("relativeShift", m_relativeShift, "Relative shift between 3- and 6-sample events, in units of APV clock / 4", int(0));
  addParam("useDB", m_useDB, "default = False. If true reads the DAQMode from SVDGlobalConfigParameters", bool(false));
  addParam("TRGSummaryName", m_objTrgSummaryName, "TRGSummary name", m_objTrgSummaryName);

  // default ModeByte settings: 10 0 10 000 (144)
}

SVDEventInfoSetterModule::~SVDEventInfoSetterModule() = default;

void SVDEventInfoSetterModule::beginRun()
{
  if (m_useDB) {
    if (!m_svdGlobalConfig.isValid()) {
      B2WARNING("No valid SVDGlobalConfigParameters for the requested IoV, simulating a 6-sample event");
      m_daqMode = 2; //6-sample event
      m_relativeShift = 0; // not needed later in reconstruction
    } else {
      m_relativeShift = m_svdGlobalConfig->getRelativeTimeShift();
      int nFrames = m_svdGlobalConfig->getNrFrames();
      if (nFrames == 3) m_daqMode = 1;
      else if (nFrames == 6) m_daqMode = 2;
      else if (nFrames == 9) m_daqMode = 3;
      else
        B2ERROR("Invalid number of frames (" << nFrames << ") in SVDGlobalConfigParamters");
    }
  }
}

void SVDEventInfoSetterModule::initialize()
{
  //Register the EventInfo in the data store
  m_svdEventInfoPtr.registerInDataStore(m_svdEventInfoName, DataStore::c_ErrorIfAlreadyRegistered);

  m_simClockState.isOptional();

// TO BE ADDED(?): some functions than can check the validity of the given parameters
}

void SVDEventInfoSetterModule::event()
{
  if (m_fixedTriggerBin >= 0 && m_fixedTriggerBin <= 3) {
    B2DEBUG(25, "using fixed triggerBin");
    m_triggerBin = m_fixedTriggerBin;
  } else {
    if (m_simClockState.isValid())
      m_triggerBin = m_simClockState->getSVDTriggerBin();
    else {
      const int triggerBinsInAPVclock = 4;
      m_triggerBin = gRandom->Integer(triggerBinsInAPVclock);
      B2DEBUG(25, "no SimClockState -> random generation of trigger bin");
    }
  }


  m_SVDModeByte.setRunType(m_runType);
  m_SVDModeByte.setEventType(m_eventType);
  m_SVDModeByte.setDAQMode(m_daqMode);
  m_SVDModeByte.setTriggerBin(m_triggerBin);
  m_SVDTriggerType.setType(m_triggerType);

  B2DEBUG(25, " triggerBin = " << m_triggerBin);

  m_svdEventInfoPtr.create();
  m_svdEventInfoPtr->setModeByte(m_SVDModeByte);
  m_svdEventInfoPtr->setMatchModeByte(m_ModeByteMatch);
  m_svdEventInfoPtr->setTriggerType(m_SVDTriggerType);
  m_svdEventInfoPtr->setMatchTriggerType(m_TriggerTypeMatch);
  m_svdEventInfoPtr->setCrossTalk(m_xTalk);
  m_svdEventInfoPtr->setRelativeShift(m_relativeShift);

  int nAPVsamples = 6;

  if (m_daqMode == 1) nAPVsamples = 3;
  else if (m_daqMode == 3) {

    StoreObjPtr<TRGSummary> storeTRGSummary(m_objTrgSummaryName);

    if (storeTRGSummary.isValid()) {
      int trgQuality = storeTRGSummary->getTimQuality();

      if (trgQuality == TRGSummary::ETimingQuality::TTYQ_FINE or trgQuality == TRGSummary::ETimingQuality::TTYQ_SFIN)
        nAPVsamples = 3;
    } else B2DEBUG(25, "DAQMode = 3, but no valid TRGSummary! We simulate a 6-sample event");
  }

  m_svdEventInfoPtr->setNSamples(nAPVsamples);

  if (nAPVsamples == 3)
    B2DEBUG(25, " relativeShift = " << m_relativeShift);

}

