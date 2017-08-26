/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/modules/TestHisto/TestHistoModule.h>

#include <TTree.h>
#include <TRandom.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TestHisto)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TestHistoModule::TestHistoModule() : CalibrationCollectorModuleNew()
{
  // Set module properties
  setDescription("Test Module for saving big data in CAF");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("entriesPerEvent", m_entriesPerEvent,
           "Number of entries that we fill into the saved tree per event. As we increase this we start storing larger amonuts of data in a smaller number of events to test the limits.",
           int(10));
}

void TestHistoModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();

  //registerObject("calib_tree",tree);
}

void TestHistoModule::collect()
{
  ;  /* code */
}

void TestHistoModule::inDefineHisto()
{
  // Data object creation --------------------------------------------------
  TTree* tree = new TTree("MyTree", "Tree with event meta data");
  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("run", &m_run);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch("hitX", &m_hitX);
  tree->Branch("hitY", &m_hitY);
  tree->Branch("hitZ", &m_hitZ);
  tree->Branch("trackX", &m_trackX);
  tree->Branch("trackY", &m_trackY);
  tree->Branch("trackZ", &m_trackZ);
  tree->Branch("chisq", &m_chisq);
  tree->Branch("pvalue", &m_pvalue);
  tree->Branch("dof", &m_dof);
}

//void TestHistoModule::beginRun()
//{
//  m_obj->constructObject("1.1");
//}

//void TestHistoModule::event()
//{
//  m_evt = m_emd->getEvent();
//  m_run = m_emd->getRun();
//  m_exp = m_emd->getExperiment();
//
//  for(int i=0; i<m_entriesPerEvent; ++i){
//    m_hitX = gRandom->Gaus();
//    m_hitY = gRandom->Gaus();
//    m_hitZ = gRandom->Gaus();
//    m_trackX = gRandom->Gaus();
//    m_trackY = gRandom->Gaus();
//    m_trackZ = gRandom->Gaus();
//    m_chisq = gRandom->Gaus();
//    m_pvalue = gRandom->Gaus();
//    m_dof = gRandom->Gaus();
//    m_obj->getObject("1.1").Fill();
//  }
//}

//void TestHistoModule::terminate()
//{
//  TDirectory* oldDir = gDirectory;
//  m_dir->cd();
//  m_obj->Write("MyCaliObj");
//  oldDir->cd();
//}
