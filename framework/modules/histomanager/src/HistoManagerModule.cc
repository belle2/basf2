//+
// File : HistoManager.cc
// Description : A module to manager histograms/ntuples/ttrees; implementation
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Jul - 2010
//-

#include <framework/modules/histomanager/HistoManagerModule.h>

#include <framework/core/Environment.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RbTuple.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HistoManager)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// Implementations
HistoManagerModule::HistoManagerModule() : Module(), m_initmain(false), m_initialized(false)
{
  // Module description
  setDescription("Module to manage histograms/Ntuples/TTrees");
  // Flag "initialization in main process"
  //  setPropertyFlags(c_InitializeInMain);
  setPropertyFlags(Module::c_HistogramManager);

  // Parameters
  addParam("HistoFileName", m_histfile, "Name of histogram output file.", string("histofile.root"));

}

HistoManagerModule::~HistoManagerModule()
{
  //  if (m_initmain) {
  if (ProcHandler::EvtProcID() == -1) {   // should be called from main proc.
    //    cout << "HistoManager:: destructor called from pid=" << ProcHandler::EvtProcID() << endl;
    if (Environment::Instance().getNumberProcesses() > 0 && ProcHandler::EvtProcID() == -1) {
      cout << "HistoManager:: adding histogram files" << endl;
      RbTupleManager::Instance().hadd();
    }
  }
}

void HistoManagerModule::initialize()
{
  RbTupleManager::Instance().init(Environment::Instance().getNumberProcesses(), m_histfile.c_str());

  m_initmain = true;
  //  cout << "HistoManager::initialization done" << endl;

}

void HistoManagerModule::beginRun()
{
  if (!m_initialized) {
    //    cout << "HistoManager:: first pass in beginRun() : proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManagerModule::endRun()
{
  if (!m_initialized) {
    //    cout << "HistoManager:: first pass in endRun(): proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManagerModule::event()
{
  if (!m_initialized) {
    //    cout << "HistoManager:: first pass in event() : proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManagerModule::terminate()
{
  if (m_initialized) {
    //    cout << "HistoManager::terminating event process : PID=" << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().terminate();
  }
}



