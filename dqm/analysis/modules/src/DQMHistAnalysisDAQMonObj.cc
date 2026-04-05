/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisDAQMonObj.h>
#include <TH1F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisDAQMonObj);

DQMHistAnalysisDAQMonObjModule::DQMHistAnalysisDAQMonObjModule()
  : DQMHistAnalysisModule()
{
  setDescription("Module to monitor DAQ information.");
  B2DEBUG(20, "DQMHistAnalysisDAQMonObj: Constructor done.");
}

DQMHistAnalysisDAQMonObjModule::~DQMHistAnalysisDAQMonObjModule()
{
}

void DQMHistAnalysisDAQMonObjModule::initialize()
{
  m_monObj = getMonitoringObject("daq");
  B2DEBUG(20, "DQMHistAnalysisDAQMonObj: initialized.");
}

void DQMHistAnalysisDAQMonObjModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisDAQMonObj: beginRun called.");
}

void DQMHistAnalysisDAQMonObjModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisDAQMonObj: event called.");
}

void DQMHistAnalysisDAQMonObjModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisDAQMonObj: endRun called.");

  auto* PXDDataSize = findHist("DAQ/PXDDataSize");
  auto* SVDDataSize = findHist("DAQ/SVDDataSize");
  auto* CDCDataSize = findHist("DAQ/CDCDataSize");
  auto* TOPDataSize = findHist("DAQ/TOPDataSize");
  auto* ARICHDataSize = findHist("DAQ/ARICHDataSize");
  auto* ECLDataSize = findHist("DAQ/ECLDataSize");
  auto* KLMDataSize = findHist("DAQ/KLMDataSize");
  auto* TRGDataSize = findHist("DAQ/TRGDataSize");
  auto* HLTDataSize = findHist("DAQ/HLTDataSize");
  auto* TotalDataSize = findHist("DAQ/TotalDataSize");

  m_monObj->setVariable("PXDEventSize", PXDDataSize ? PXDDataSize->GetMean() : 0,
                        PXDDataSize ? PXDDataSize->GetMeanError() : -1);
  m_monObj->setVariable("SVDEventSize", SVDDataSize ? SVDDataSize->GetMean() : 0,
                        SVDDataSize ? SVDDataSize->GetMeanError() : -1);
  m_monObj->setVariable("CDCEventSize", CDCDataSize ? CDCDataSize->GetMean() : 0,
                        CDCDataSize ? CDCDataSize->GetMeanError() : -1);
  m_monObj->setVariable("TOPEventSize", TOPDataSize ? TOPDataSize->GetMean() : 0,
                        TOPDataSize ? TOPDataSize->GetMeanError() : -1);
  m_monObj->setVariable("ARICHEventSize", ARICHDataSize ? ARICHDataSize->GetMean() : 0,
                        ARICHDataSize ? ARICHDataSize->GetMeanError() : -1);
  m_monObj->setVariable("ECLEventSize", ECLDataSize ? ECLDataSize->GetMean() : 0,
                        ECLDataSize ? ECLDataSize->GetMeanError() : -1);
  m_monObj->setVariable("KLMEventSize", KLMDataSize ? KLMDataSize->GetMean() : 0,
                        KLMDataSize ? KLMDataSize->GetMeanError() : -1);
  m_monObj->setVariable("TRGEventSize", TRGDataSize ? TRGDataSize->GetMean() : 0,
                        TRGDataSize ? TRGDataSize->GetMeanError() : -1);
  m_monObj->setVariable("HLTEventSize", HLTDataSize ? HLTDataSize->GetMean() : 0,
                        HLTDataSize ? HLTDataSize->GetMeanError() : -1);
  m_monObj->setVariable("TotalEventSize", TotalDataSize ? TotalDataSize->GetMean() : 0,
                        TotalDataSize ? TotalDataSize->GetMeanError() : -1);
}

void DQMHistAnalysisDAQMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

