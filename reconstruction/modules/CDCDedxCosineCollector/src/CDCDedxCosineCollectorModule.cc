/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxCosineCollector/CDCDedxCosineCollectorModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxCosineCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxCosineCollectorModule::CDCDedxCosineCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx cosine calibration");

  // Parameter definitions

}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxCosineCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<CDCDedxTrack>::required();

  // Data object creation
  auto ttree = new TTree("dedxTree", "Tree with dE/dx information");
  ttree->Branch<int>("event", &m_evt);
  ttree->Branch<int>("run", &m_run);
  ttree->Branch<int>("exp", &m_exp);
  ttree->Branch<int>("pid", &m_procId);

  ttree->Branch<double>("dedx", &m_dedx);
  ttree->Branch<double>("costh", &m_costh);
  ttree->Branch<int>("nhits", &m_nhits);

  // Data object registration
  registerObject<TTree>("tree", ttree);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------

void CDCDedxCosineCollectorModule::collect()
{
  StoreObjPtr<EventMetaData> emd;
  StoreArray<CDCDedxTrack> tracks;

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  for (auto track : tracks) {
    m_dedx = track.getTruncatedMean();
    m_costh = track.getCosTheta();
    m_nhits = track.getNLayerHits();

    // Data object access and filling
    getObject<TTree>("tree").Fill();
  }
}
