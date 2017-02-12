/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/modules/CaTest/CaTestModule.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TH1F.h>
#include <TTree.h>
#include <TFile.h>
#include <TRandom.h>
#include <unistd.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CaTest)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CaTestModule::CaTestModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration Test Module");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("spread", m_spread,
           "Spread of gaussian (mean=42) filling of test histogram (range=<0,100>) - probability of algo iterations depend on it", int(17.));
  addParam("wait", m_wait,
           "Time in microseconds to usleep during prepare() method before starting.", int(0));
}

void CaTestModule::prepare()
{

  StoreObjPtr<EventMetaData>::required();

  usleep(m_wait);

  // Data object creation --------------------------------------------------
  auto histo1 = new TH1F("histo", "Test histogram, which mean value should be found by test calibration algo", 100, 0., 100.);
  auto ttree = new TTree("eventData", "Tree with event meta data");
  ttree->Branch<int>("event", &m_evt);
  ttree->Branch<int>("run", &m_run);
  ttree->Branch<int>("exp", &m_exp);
  ttree->Branch<int>("pid", &m_procId);
  auto mmille = new MilleData();

  // Data object registration ----------------------------------------------
  registerObject<TH1F>("histogram1", histo1);
  registerObject<TTree>("tree", ttree);
  registerObject<MilleData>("test_mille", mmille);

}

void CaTestModule::collect()
{
  static int nevents = 0;
  StoreObjPtr<EventMetaData> emd;

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  // Data object access and filling ----------------------------------------
  getObject<TH1F>("histogram1").Fill(gRandom->Gaus(42., m_spread));
  getObject<TTree>("tree").Fill();

  auto& mille = getObject<MilleData>("test_mille");
  if (!mille.isOpen()) mille.open(
      to_string(emd->getExperiment()) + "-" + to_string(emd->getRun()) + "-sevt-" + to_string(emd->getEvent()) + "-pid" + std::to_string(
        ProcHandler::EvtProcID()) + ".mille");

  //if (mille.isOpen() && nevents && nevents % 100 == 0)
  //  mille.close();

  ++nevents;
}
