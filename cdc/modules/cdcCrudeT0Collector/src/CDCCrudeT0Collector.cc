/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcCrudeT0Collector/CDCCrudeT0Collector.h"
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>
#include <TH1F.h>
#include <framework/pcore/ProcHandler.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
REG_MODULE(CDCCrudeT0Collector);

CDCCrudeT0CollectorModule::CDCCrudeT0CollectorModule() : CalibrationCollectorModule()
{
  setDescription("Collector for crude t0");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("ADCCut", m_adcMin, "threshold of ADC", m_adcMin);

}

void CDCCrudeT0CollectorModule::prepare()
{
  describeProcess("CDCCrudeT0Collector::prepare");
  std::string objectName = "tree";
  TTree* tree = new TTree(objectName.c_str(), "");
  tree->Branch<unsigned short>("lay", &m_lay);
  tree->Branch<unsigned short>("wire", &m_wire);
  tree->Branch<unsigned short>("tdc", &m_tdc);
  registerObject<TTree>(objectName, tree);
}

void CDCCrudeT0CollectorModule::startRun()
{
  describeProcess("CDCCrudeT0Collector::startRun()");
}

void CDCCrudeT0CollectorModule::closeRun()
{
  describeProcess("CDCCrudeT0Collector::closeRun()");
}

void CDCCrudeT0CollectorModule::collect()
{
  describeProcess("CDCCrudeT0Collector::collect()");
  auto tree = getObjectPtr<TTree>("tree");
  for (const auto& hit : m_cdcHits) {
    WireID wireid(hit.getID());
    m_lay = wireid.getICLayer();
    m_wire = wireid.getIWire();
    m_tdc = hit.getTDCCount();
    if (hit.getADCCount() > m_adcMin) {
      tree->Fill();
    }
  }
}

void CDCCrudeT0CollectorModule::finish()
{
  describeProcess("CDCCrudeT0Collector::finish()");
}

void CDCCrudeT0CollectorModule::describeProcess(string functionName)
{
  B2DEBUG(100, "Running " + functionName + " function from a Process of type " + ProcHandler::getProcessName()
          + "\nParallel Processing Used = " + to_string(ProcHandler::parallelProcessingUsed())
          + "\nThis EvtProcID Id = " + to_string(ProcHandler::EvtProcID())
          + "\nThe gDirectory is " + gDirectory->GetPath());
}
