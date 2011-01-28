/*
 * pEventProcessor.cc
 *
 *  Created on: Oct 26, 2010
 *      Author: molland
 */

#include <framework/pcore/pEventProcessor.h>

using namespace std;
using namespace Belle2;

pEventProcessor::pEventProcessor(PathManager& pathManager) : EventProcessor(pathManager)
{
  procHandler = new ProcHandler();
}


pEventProcessor::~pEventProcessor()
{

}


void pEventProcessor::process(PathPtr spath)
{
  if (spath->getModules().size() == 0) return;

  // 0. If nprocess is 0, pass control to kbasf2::process()

  if (m_nproc == 0) {   // Single process -> fall back to kbasf2
    //    process ( spath, maxev );
    process(spath);
    return;
  }

  //  Multiprocess case

  // 1. Initialization
  ModulePtrList modulelist = m_pathManager.buildModulePathList(spath);
  processInitialize(modulelist);

  // 2. Scan modules and find input module with multi-process support

  m_input_list = ModuleManager::Instance().getModulesByProperties
                 (modulelist,
                  Module::c_Input | Module::c_ParallelProcessingCertified);

  for (ModulePtrList::const_iterator listIter = m_input_list.begin();
       listIter != m_input_list.end(); listIter++) {
    Module* module = listIter->get();
    procHandler->init_EvtServer();
    if (procHandler->isEvtServer()) {   // In event server process
      ((pEventServer*)module)->event_server();
      exit(0);
    }
  }

  // 3. Scan modules and find output modules with multi-process support
  m_output_list = ModuleManager::Instance().getModulesByProperties
                  (modulelist,
                   Module::c_Output | Module::c_ParallelProcessingCertified);
  int nout = 0;
  for (ModulePtrList::const_iterator listIter = m_output_list.begin();
       listIter != m_output_list.end(); listIter++) {
    Module* module = listIter->get();
    procHandler->init_OutServer(nout++);
    if (procHandler->isOutputSrv()) {   // In output server process
      ((pOutputServer*)module)->output_server();
      exit(0);
    }
  }

  // 4. fork event processes
  procHandler->init_EvtProc(m_nproc);
  if (procHandler->isEvtProc()) {
    processCore(spath, modulelist);
    processTerminate(modulelist);
    exit(0);
  }

  // 5. Main process
  if (procHandler->isFramework()) {
    procHandler->wait_processes();
  }
}

void pEventProcessor::nprocess(int nproc)
{
  m_nproc = nproc;
}

int pEventProcessor::nprocess(void)
{
  return m_nproc;
}

