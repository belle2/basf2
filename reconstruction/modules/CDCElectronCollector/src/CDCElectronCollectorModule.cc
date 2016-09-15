/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCElectronCollector/CDCElectronCollectorModule.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <TH1F.h>
#include <TTree.h>
#include <TFile.h>
#include <TRandom.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCElectronCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCElectronCollectorModule::CDCElectronCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx electron calibration");

  // Parameter definitions

}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCElectronCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<CDCDedxTrack>::required();

  // Data object creation
  auto gains = new TH1F("gains", "Run gains", 200, 0.0, 200.0);
  auto ttree = new TTree("eventData", "Tree with event meta data");
  ttree->Branch<int>("event", &m_evt);
  ttree->Branch<int>("run", &m_run);
  ttree->Branch<int>("exp", &m_exp);
  ttree->Branch<int>("pid", &m_procId);

  // Data object registration
  registerObject<TH1F>("gains", gains);
  registerObject<TTree>("tree", ttree);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------

void CDCElectronCollectorModule::collect()
{
  static int nevents = 0;
  StoreObjPtr<EventMetaData> emd;
  StoreArray<CDCDedxTrack> tracks;

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  for (auto track : tracks) {
    m_dedx = track.getTruncatedMean();
    getObject<TH1F>("gains").Fill(m_dedx);
  }

  // Data object access and filling
  getObject<TTree>("tree").Fill();

  ++nevents;
}
