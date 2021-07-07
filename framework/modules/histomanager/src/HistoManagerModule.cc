/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
HistoManagerModule::HistoManagerModule() : Module(), m_initmain(false), m_tupleManagerInitialized(false)
{
  // Module description
  setDescription("Manage histograms/Ntuples/TTrees for modules inheriting from the HistoModule class. See https://confluence.desy.de/display/BI/Software+HistogramManagement for details.");
  setPropertyFlags(Module::c_HistogramManager);

  // Parameters
  addParam("histoFileName", m_histoFileName, "Name of histogram output file.", string("histofile.root"));
  addParam("workDirName", m_workdir, "Name of working directory.", string("."));

}

HistoManagerModule::~HistoManagerModule() = default;

void HistoManagerModule::initialize()
{
  RbTupleManager::Instance().init(Environment::Instance().getNumberProcesses(), m_histoFileName.c_str(), m_workdir.c_str());

  m_initmain = true;
  //  cout << "HistoManager::initialization done" << endl;

}

void HistoManagerModule::beginRun()
{
  if (!m_tupleManagerInitialized) {
    //    cout << "HistoManager:: first pass in beginRun() : proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_tupleManagerInitialized = true;
  }
}

void HistoManagerModule::endRun()
{
  if (!m_tupleManagerInitialized) {
    //    cout << "HistoManager:: first pass in endRun(): proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_tupleManagerInitialized = true;
  }
}

void HistoManagerModule::event()
{
  if (!m_tupleManagerInitialized) {
    //    cout << "HistoManager:: first pass in event() : proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_tupleManagerInitialized = true;
  }
}

void HistoManagerModule::terminate()
{
  if (m_tupleManagerInitialized) {
    //    cout << "HistoManager::terminating event process : PID=" << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().terminate();
  }
}
