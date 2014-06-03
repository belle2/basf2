/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>
#include <framework/logging/LogConfig.h>

#include <boost/filesystem/path.hpp>

#include <iostream>
#include <cstdlib>

using namespace Belle2;
using namespace std;
namespace fs = boost::filesystem;


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
  m_logLevelOverride(LogConfig::c_Default),
  m_visualizeDataFlow(false),
  m_noStats(false),
  m_dryRun(false)
{
  // Check for environment variables set by setuprel
  const char* envarReleaseDir = getenv("BELLE2_RELEASE_DIR");
  const char* envarLocalDir = getenv("BELLE2_LOCAL_DIR");
  if (!envarReleaseDir and !envarLocalDir) {
    B2FATAL("The basf2 environment is not set up. Please execute the 'setuprel' script first.");
  }

  const char* envarSubDir = getenv("BELLE2_SUBDIR");
  if (!envarSubDir) {
    B2FATAL("The environment variable BELLE2_SUBDIR is not set. Please execute the 'setuprel' script first.");
  }

  const char* envarExtDir = getenv("BELLE2_EXTERNALS_DIR");
  if (!envarExtDir) {
    B2FATAL("The environment variable BELLE2_EXTERNALS_DIR is not set. Please execute the 'setuprel' script first.");
  }

  // add module directories for current build options
  if (envarLocalDir) {
    const string localModules = (fs::path(envarLocalDir) / "modules" / envarSubDir).string();
    ModuleManager::Instance().addModuleSearchPath(localModules);
  }

  if (envarReleaseDir) {
    const string centralModules = (fs::path(envarReleaseDir) / "modules" / envarSubDir).string();
    ModuleManager::Instance().addModuleSearchPath(centralModules);
  }

  //set path to external software
  setExternalsPath(envarExtDir);

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
