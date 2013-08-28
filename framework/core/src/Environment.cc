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

void Environment::setJobInformation(boost::shared_ptr<Path> path)
{
  const std::list<ModulePtr>& modules = path->getModules();

  for (ModulePtr m : modules) {
    string name = m->getName();
    if (name == "RootInput") {
      std::vector<std::string> inputFiles = Environment::Instance().getInputFilesOverride();
      if (inputFiles.empty()) {
        string inputFileName = m->getParam<string>("inputFileName").getValue();
        if (!inputFileName.empty())
          inputFiles.push_back(inputFileName);
        else
          inputFiles = m->getParam<vector<string> >("inputFileNames").getValue();
      }

      for (string file : inputFiles) {
        m_jobInfoOutput += "INPUT FILE: " + file + "\n";
      }
    } else if (name == "RootOutput") {
      std::string out = Environment::Instance().getOutputFileOverride();
      if (out.empty())
        out = m->getParam<string>("outputFileName").getValue();
      m_jobInfoOutput += "OUTPUT FILE: " + out + "\n";
    }
  }
}

void Environment::printJobInformation() const
{
  cout << m_jobInfoOutput;
}
