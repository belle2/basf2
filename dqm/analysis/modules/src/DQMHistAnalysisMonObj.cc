/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisMonObj.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TH1F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisMonObj);

DQMHistAnalysisMonObjModule::DQMHistAnalysisMonObjModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Example module for making MonitoringObject in DQMHistAnalysis module");
  setPropertyFlags(c_ParallelProcessingCertified);
}

DQMHistAnalysisMonObjModule::~DQMHistAnalysisMonObjModule()
{
}

void DQMHistAnalysisMonObjModule::initialize()
{
  // make monitoring object related to this module (use arich as an example)
  // if monitoring object already exists this will return pointer to it
  m_monObj = getMonitoringObject("arich");

  // make canvases to be added to MonitoringObject
  m_c_main = new TCanvas("main");
  m_c_mask = new TCanvas("mask");

  // add canvases to MonitoringObject
  m_monObj->addCanvas(m_c_main);
  m_monObj->addCanvas(m_c_mask);
}

void DQMHistAnalysisMonObjModule::beginRun()
{
}

void DQMHistAnalysisMonObjModule::event()
{
  // can put the analysis code here or in endRun() function
  // for the start tests we will store output only end of run so better to put code there
}

void DQMHistAnalysisMonObjModule::endRun()
{

  // get existing histograms produced by DQM modules
  TH1* hits = findHist("ARICH/hitsPerEvent");
  TH1* bits = findHist("ARICH/bits");

  // set the content of main canvas
  m_c_main->Clear(); // clear existing content
  m_c_main->Divide(2, 1);
  m_c_main->cd(1);
  if (hits) hits->Draw();
  m_c_main->cd(2);
  if (bits) bits->Draw();

  // set values of monitoring variables (if variable already exists this will change its value, otherwise it will insert new variable)
  // with error (also asymmetric error can be used as m_monObj->setVariable(name, value, upError, downError))
  m_monObj->setVariable("hitsPerEvent", hits ? hits->GetMean() : 0, hits ? hits->GetMeanError() : -1);
  // without error
  m_monObj->setVariable("bitsMean", bits ? bits->GetMean() : 0);

  // set string variable
  m_monObj->setVariable("mode", "nominal");


  B2DEBUG(20, "DQMHistAnalysisMonObj : endRun called");
}

void DQMHistAnalysisMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}
