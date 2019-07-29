/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio, Giulia Casarosa                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDDataFormatCheckModule.h>
#include <svd/dataobjects/SVDEventInfo.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDataFormatCheck)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDataFormatCheckModule::SVDDataFormatCheckModule() : Module()
{
  setDescription("Checks the SVD data format: ");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDEventInfo", m_svdEventInfoName,
           "SVDEventInfo name", string(""));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string("SVDShaperDigits"));
  addParam("DAQDiagnostics", m_storeDAQName,
           "DAQDiagnostics collection name", string("SVDDAQDiagnostics"));
  addParam("maxProblematicEvents", m_maxProblematicEvts,
           "maximum number of problematic events to display WARNING", int(10));
}


SVDDataFormatCheckModule::~SVDDataFormatCheckModule()
{
}


void SVDDataFormatCheckModule::initialize()
{

  m_evtMetaData.isRequired();
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  //there only if reconstructing data
  m_storeDAQ.isOptional(m_storeDAQName);

  B2DEBUG(1, " COLLECTIONS:");
  B2DEBUG(1, " -->  Digits:             " << m_storeShaperDigitsName);
  B2DEBUG(1, " -->  Diagnostic:         " << m_storeDAQName);
}

void SVDDataFormatCheckModule::beginRun()
{

  m_expNumber = m_evtMetaData->getExperiment();
  m_runNumber = m_evtMetaData->getRun();

  m_shutUpWarnings = false;
  m_problematicEvtsCounter = 0;
  m_stripEvtsCounter = 0;
  m_evtsCounter = 0;
  m_nLocalRunEvts = 0;
  m_nNoZSEvts = 0;
  m_nBadTBEvts = 0;
  m_n1samples = 0;
  m_n3samples = 0;
  m_n6samples = 0;

}


void SVDDataFormatCheckModule::event()
{

  int evtNumber = m_evtMetaData->getEvent();

  StoreObjPtr<SVDEventInfo> storeSVDEvtInfo(m_svdEventInfoName);
  SVDModeByte modeByte = storeSVDEvtInfo->getModeByte();

  bool isProblematic = false;

  // If no digits, nothing to do
  if (!m_storeShaper || !m_storeShaper.getEntries()) {

    if (m_evtsCounter < m_maxProblematicEvts)
      B2INFO("SVDDataFormatCheck: no " << m_storeShaperDigitsName << " in event " << evtNumber << " of exp " << m_expNumber << ", run " <<
             m_runNumber);
    m_evtsCounter++;
    return;
  }

  m_stripEvtsCounter++;

  //checking the number of acquired samples per APV
  int daqMode = (int) modeByte.getDAQMode();
  //0 -> 1-sample
  //1 -> 3-sample
  //2 -> 6-sample
  if (daqMode == 2)
    m_n6samples++;

  if (daqMode == 1) {
    m_n3samples++;
    isProblematic = true;
    if (!m_shutUpWarnings)
      B2WARNING("SVDDataFormatCheck: the event " << evtNumber << " of exp " << m_expNumber << ", run " << m_runNumber <<
                " is apparently taken with 3-sample mode, this is not expected. [daqMode = " << daqMode << "]");
  }

  if (daqMode == 0) {
    m_n1samples++;
    isProblematic = true;
    if (!m_shutUpWarnings)
      B2WARNING("SVDDataFormatCheck: the event " << evtNumber << " of exp " << m_expNumber << ", run " << m_runNumber <<
                " is apparently taken with 1-sample mode, this is not expected. [daqMode = " << daqMode << "]");
  }

  int evtType = (int) modeByte.getEventType();
  //0 -> global run
  //1 -> local run
  if (evtType != 0) { //only global runs are expected
    m_nLocalRunEvts++;
    isProblematic = true;
    if (!m_shutUpWarnings)
      B2WARNING("SVDDataFormatCheck: the event " << evtNumber << " of exp " << m_expNumber << ", run " << m_runNumber <<
                " is apparently taken as Local Run, this is not expected. [evtType = " << evtType << "]");
  }

  int runType = (int) modeByte.getRunType();
  //0 -> raw
  //1 -> transparent
  //2 -> zero suppressed
  //3 -> zs + hit time
  if (runType != 2) { //only zero suppressed events are expected
    m_nNoZSEvts++;
    isProblematic = true;
    if (!m_shutUpWarnings)
      B2WARNING("SVDDataFormatCheck: the event " << evtNumber << " of exp " << m_expNumber << ", run " << m_runNumber <<
                " is apparently not taken as ZeroSuppressed, this is not expected. [runType = " << runType << "]");
  }

  int triggerBin = modeByte.getTriggerBin();
  //between 0 and 3
  if (triggerBin < 0 || triggerBin > 3) {
    m_nBadTBEvts++;
    isProblematic = true;
    if (!m_shutUpWarnings)
      B2WARNING("SVDDataFormatCheck: the event " << evtNumber << " of exp " << m_expNumber << ", run " << m_runNumber <<
                " is apparently not with an unexpected trigger bin = " << triggerBin);
  }

  if (isProblematic)
    m_problematicEvtsCounter++;

  if (m_problematicEvtsCounter > m_maxProblematicEvts)
    m_shutUpWarnings = true;

}


void SVDDataFormatCheckModule::endRun()
{

  B2RESULT("SVDDataFormatCheck counters:");
  B2RESULT("total number of events with at least one strip = " << m_stripEvtsCounter);
  B2RESULT("total number of apparently problematic events = " << m_problematicEvtsCounter);
  if (m_nLocalRunEvts > 0)
    B2RESULT("total number local-run strips = " <<  m_nLocalRunEvts);
  if (m_nNoZSEvts > 0)
    B2RESULT("total number of NOT zero-suppressed strips = " <<  m_nNoZSEvts);
  if (m_nBadTBEvts > 0)
    B2RESULT("total number of strips with wrong TB = " <<  m_nBadTBEvts);
  if (m_n1samples > 0 || m_n3samples > 0) {
    B2RESULT("total number of 1-sample strips = " <<  m_n1samples);
    B2RESULT("total number of 3-sample strips = " <<  m_n3samples);
    B2RESULT("total number of 6-sample strips = " <<  m_n6samples);
  }
}


void SVDDataFormatCheckModule::terminate()
{
}




































