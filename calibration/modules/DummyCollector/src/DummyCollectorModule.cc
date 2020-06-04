/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
