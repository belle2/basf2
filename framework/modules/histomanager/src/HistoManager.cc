//+
// File : HistoManager.cc
// Description : A module to manager histograms/ntuples/ttrees; implementation
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Jul - 2010
//-

#include <framework/core/ModuleManager.h>

#include <framework/modules/histomanager/HistoManager.h>

#include <framework/core/Framework.h>
#include <framework/pcore/ProcHandler.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HistoManager, "HistoManager")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// Implementations
HistoManager::HistoManager() : Module(), m_initmain(false), m_initialized(false)
{
  // Module description
  setDescription("Module to manage histograms/Ntuples/TTrees");

  // Parameters
  addParam("HistoFileName", m_histfile, string("histofile.root"),
           "Histogram File Name");
}

HistoManager::~HistoManager()
{
  if (m_initmain) {
    cout << "HistoManager:: destructor called from pid=" << ProcHandler::EvtProcID() << endl;
    if (Framework::nprocess() > 0 && ProcHandler::EvtProcID() == -1) {
      cout << "HistoManager:: adding histogram files" << endl;
      RbTupleManager::Instance().hadd();
    }
  }
}

void HistoManager::initialize()
{
  RbTupleManager::Instance().init(Framework::nprocess(), m_histfile.c_str());

  m_initmain = true;
  cout << "HistoManager::initialization done" << endl;

}

void HistoManager::beginRun()
{
  if (!m_initialized) {
    cout << "HistoManager:: first pass" << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManager::endRun()
{
  if (!m_initialized) {
    cout << "HistoManager:: first pass" << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManager::event()
{
  if (!m_initialized) {
    cout << "HistoManager:: first pass" << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void HistoManager::terminate()
{
  if (m_initialized) {
    cout << "HistoManager::terminating event process : PID=" << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().terminate();
  }
}



