/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Path.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/Module.h>

#include <framework/modules/rootio/RootInputModule.h>
#include <framework/modules/rootio/RootOutputModule.h>

#include <boost/foreach.hpp>

#include <iostream>

using namespace Belle2;
using namespace std;


Environment& Environment::Instance()
{
  static Environment instance;
  return instance;
}


const list<string>& Environment::getModuleSearchPaths() const
{
  return ModuleManager::Instance().getModuleSearchPaths();
}


//============================================================================
//                              Private methods
//============================================================================

Environment::Environment() :
  m_numberProcesses(0),
  m_steering(""),
  m_numberEventsOverride(0),
  m_inputFilesOverride(),
  m_outputFileOverride(""),
  m_numberProcessesOverride(-1),
  m_visualizeDataFlow(false),
  m_noStats(false),
  m_dryRun(false)
{

}


Environment::~Environment()
{
}

void Environment::printJobInformation() const
{
  assert(m_mainPath);
  const std::list<ModulePtr>& modules = m_mainPath->getModules();

  BOOST_FOREACH(ModulePtr m, modules) {
    string name = m->getName();
    if (name == "RootInput") {
      const RootInputModule* input = static_cast<RootInputModule*>(m.get());
      const vector<string>& inputs = input->getInputFiles();
      for (unsigned int i = 0; i < inputs.size(); i++) {
        cout << "INPUT FILE: " << inputs[i] << "\n";
      }
    } else if (name == "RootOutput") {
      const RootOutputModule* output = static_cast<const RootOutputModule*>(m.get());
      string out = output->getOutputFile();
      cout << "OUTPUT FILE: " << out << "\n";
    }
  }
}
