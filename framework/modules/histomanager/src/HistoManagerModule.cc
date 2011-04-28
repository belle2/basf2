//+
// File : HistoManager.cc
// Description : A module to manager histograms/ntuples/ttrees; implementation
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Jul - 2010
//-

#include <framework/modules/histomanager/HistoManagerModule.h>
#include <framework/core/Environment.h>

#include <framework/core/Framework.h>
#include <framework/pcore/ProcHandler.h>

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

  // Parameters
  addParam("HistoFileName", m_histfile, "Histogram File Name", string("histofile.root"));
}

HistoManagerModule::~HistoManagerModule()
{
  if (m_initmain) {
    cout << "HistoManager:: destructor called from pid=" << ProcHandler::EvtProcID() << endl;
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
  cout << "HistoManager::initialization done" << endl;

}

void HistoManagerModule::beginRun()
{
  if (!m_initialized) {
    cout << "HistoManager:: first pass" << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManagerModule::endRun()
{
  if (!m_initialized) {
    cout << "HistoManager:: first pass" << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManagerModule::event()
{
  if (!m_initialized) {
    cout << "HistoManager:: first pass" << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManagerModule::terminate()
{
  if (m_initialized) {
    cout << "HistoManager::terminating event process : PID=" << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().terminate();
  }
}



