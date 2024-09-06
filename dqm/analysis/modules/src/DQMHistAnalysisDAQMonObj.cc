/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisDAQMonObj.h>

// DAQ geometry
#include <TROOT.h>
#include <TEllipse.h>
#include <TF1.h>
#include <TLine.h>
#include <TStyle.h>

#include <numeric>

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

  auto* PXDDataSize = (TH1F*)findHist("DAQ/PXDDataSize");
  auto* SVDDataSize = (TH1F*)findHist("DAQ/SVDDataSize");
  auto* CDCDataSize = (TH1F*)findHist("DAQ/CDCDataSize");
  auto* TOPDataSize = (TH1F*)findHist("DAQ/TOPDataSize");
  auto* ARICHDataSize = (TH1F*)findHist("DAQ/ARICHDataSize");
  auto* ECLDataSize = (TH1F*)findHist("DAQ/ECLDataSize");
  auto* KLMDataSize = (TH1F*)findHist("DAQ/KLMDataSize");
  auto* TRGDataSize = (TH1F*)findHist("DAQ/TRGDataSize");
  auto* HLTDataSize = (TH1F*)findHist("DAQ/HLTDataSize");
  auto* TotalDataSize = (TH1F*)findHist("DAQ/TotalDataSize");

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

