/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxRunGainCollector/CDCDedxRunGainCollectorModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxRunGainCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxRunGainCollectorModule::CDCDedxRunGainCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx run gain calibration");

  // Parameter definitions

}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxRunGainCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<CDCDedxTrack>::required();

  // Data object creation
  auto means = new TH1F("means", "CDC dE/dx truncated means", 100, 20, 70);
  auto ttree = new TTree("eventData", "Run numbers");
  ttree->Branch<int>("event", &m_evt);
  ttree->Branch<int>("run", &m_run);
  ttree->Branch<int>("exp", &m_exp);

  // Data object registration
  registerObject<TH1F>("means", means);
  registerObject<TTree>("tree", ttree);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------

void CDCDedxRunGainCollectorModule::collect()
{
  StoreObjPtr<EventMetaData> emd;
  StoreArray<CDCDedxTrack> tracks;

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  getObject<TTree>("tree").Fill();

  for (auto track : tracks) {
    if (track.getNLayerHits() < 10) continue;
    getObject<TH1F>("means").Fill(track.getTruncatedMean());
  }
}
