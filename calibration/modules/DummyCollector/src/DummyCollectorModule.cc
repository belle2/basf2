/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <calibration/modules/DummyCollector/DummyCollectorModule.h>

#include <TTree.h>
#include <TH1I.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DummyCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DummyCollectorModule::DummyCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Dummy module for running when you don't really need output.");
}

void DummyCollectorModule::prepare()
{
  auto hist = new TH1I("histogram", "Number of times hist->Fill() was called.", 1, 0., 1.);
  registerObject<TH1I>("DummyHist", hist);
}

void DummyCollectorModule::collect()
{
  getObjectPtr<TH1I>("DummyHist")->Fill(0.5);
}
