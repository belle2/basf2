//+
// File : pFramework.cc
// Description : Additional functions for parallel processing
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - May - 2010
//-

#include <framework/core/Framework.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/logging/LogConnectionIOStream.h>
#include <framework/logging/LogConnectionTxtFile.h>

#include <framework/pcore/pFramework.h>

using namespace std;
using namespace Belle2;

using namespace boost::python;

// Constructor/Destructor

pFramework::pFramework(void)
{
  m_pEventProcessor = new pEventProcessor(*m_pathManager);
}

pFramework::~pFramework(void)
{
  delete m_pEventProcessor;
}

//void pFramework::process_parallel ( PathPtr spath, unsigned long maxev )
void pFramework::process_parallel(PathPtr spath)
{
  m_pEventProcessor->process_parallel(spath);
}

// Examine/Modify m_nproc vaiable from outside
int pFramework::nprocess(void)
{
  return pEventProcessor::m_nproc;
}

void pFramework::nprocess(int nproc)
{
  m_pEventProcessor->m_nproc = nproc;
}


// Python hooks

void pFramework::exposePythonAPI()
{

  //  cout <<  "Expose Python API called......" << endl;

  //Overloaded process methods
  ModulePtr(Framework::*registerModule1)(string) = &Framework::registerModule;
  ModulePtr(Framework::*registerModule2)(string, string) = &Framework::registerModule;
  void(pFramework::*set_nprocess)(int) = &pFramework::nprocess;

  // Expose pFramework class
  class_<pFramework>("pFramework")
  .def("process", &pFramework::process_parallel)
  .def("set_nprocess", set_nprocess)

  .def("add_module_search_path", &Framework::addModuleSearchPath)
  .def("list_module_search_paths", &Framework::getModuleSearchPathsPython)
  .def("list_available_modules", &Framework::getAvailableModulesPython)
  .def("register_module", registerModule1)
  .def("register_module", registerModule2)
  .def("list_registered_modules", &Framework::getRegisteredModulesPython)
  .def("create_path", &Framework::createPath)
  .def("log_to_shell", &Framework::setLoggingToShell)
  .def("log_to_txtfile", &Framework::setLoggingToTxtFile)
  .def("log_statistics", &Framework::getLogStatisticPython)
  ;

  //  Framework::exposePythonAPI();
}


