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
#include <cdc/geometry/DAQGeometryPar.h>

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

  m_PXDDataSize = (TH1F*)findHist("DAQ/PXDDataSize");
  m_SVDDataSize = (TH1F*)findHist("DAQ/SVDDataSize");
  m_CDCDataSize = (TH1F*)findHist("DAQ/CDCDataSize");
  m_TOPDataSize = (TH1F*)findHist("DAQ/TOPDataSize");
  m_ARICHDataSize = (TH1F*)findHist("DAQ/ARICHDataSize");
  m_ECLDataSize = (TH1F*)findHist("DAQ/ECLDataSize");
  m_KLMDataSize = (TH1F*)findHist("DAQ/KLMDataSize");
  m_TRGDataSize = (TH1F*)findHist("DAQ/TRGDataSize");
  m_HLTDataSize = (TH1F*)findHist("DAQ/HLTDataSize");
  m_TotalDataSize = (TH1F*)findHist("DAQ/TotalDataSize");

  m_monObj->setVariable("PXDEventSize", m_PXDDataSize ? m_PXDDataSize->GetMean() : 0,
                        m_PXDDataSize ? m_PXDDataSize->GetMeanError() : -1);
  m_monObj->setVariable("SVDEventSize", m_SVDDataSize ? m_SVDDataSize->GetMean() : 0,
                        m_SVDDataSize ? m_SVDDataSize->GetMeanError() : -1);
  m_monObj->setVariable("CDCEventSize", m_CDCDataSize ? m_CDCDataSize->GetMean() : 0,
                        m_CDCDataSize ? m_CDCDataSize->GetMeanError() : -1);
  m_monObj->setVariable("TOPEventSize", m_TOPDataSize ? m_TOPDataSize->GetMean() : 0,
                        m_TOPDataSize ? m_TOPDataSize->GetMeanError() : -1);
  m_monObj->setVariable("ARICHEventSize", m_ARICHDataSize ? m_ARICHDataSize->GetMean() : 0,
                        m_ARICHDataSize ? m_ARICHDataSize->GetMeanError() : -1);
  m_monObj->setVariable("ECLEventSize", m_ECLDataSize ? m_ECLDataSize->GetMean() : 0,
                        m_ECLDataSize ? m_ECLDataSize->GetMeanError() : -1);
  m_monObj->setVariable("KLMEventSize", m_KLMDataSize ? m_KLMDataSize->GetMean() : 0,
                        m_KLMDataSize ? m_KLMDataSize->GetMeanError() : -1);
  m_monObj->setVariable("TRGEventSize", m_TRGDataSize ? m_TRGDataSize->GetMean() : 0,
                        m_TRGDataSize ? m_TRGDataSize->GetMeanError() : -1);
  m_monObj->setVariable("HLTEventSize", m_HLTDataSize ? m_HLTDataSize->GetMean() : 0,
                        m_HLTDataSize ? m_HLTDataSize->GetMeanError() : -1);
  m_monObj->setVariable("TotalEventSize", m_TotalDataSize ? m_TotalDataSize->GetMean() : 0,
                        m_TotalDataSize ? m_TotalDataSize->GetMeanError() : -1);
}

void DQMHistAnalysisDAQMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

