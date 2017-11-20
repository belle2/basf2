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
#include <framework/core/InputController.h>

#include <boost/filesystem/path.hpp>
#include <memory>

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

unsigned int Environment::getNumberOfEvents() const
{
  // for EventInfoSetter, -n is already taken into account
  if (m_mcEvents != 0)
    return m_mcEvents;

  unsigned int numEventsFromInput = InputController::numEntries();
  unsigned int numEventsFromArgument = getNumberEventsOverride();
  if (numEventsFromArgument != 0
      && (numEventsFromInput == 0 || numEventsFromArgument < numEventsFromInput))
    return numEventsFromArgument;
  else
    return numEventsFromInput;
}


//============================================================================
//                              Private methods
//============================================================================

Environment::Environment() :
  m_numberProcesses(0),
  m_steering(""),
  m_numberEventsOverride(0),
  m_inputFilesOverride(),
  m_entrySequencesOverride(),
  m_outputFileOverride(""),
  m_numberProcessesOverride(-1),
  m_logLevelOverride(LogConfig::c_Default),
  m_visualizeDataFlow(false),
  m_noStats(false),
  m_dryRun(false),
  m_mcEvents(0),
  m_run(-1),
  m_experiment(-1),
  m_skipNEvents(0)
{
  // Check for environment variables set by setuprel
  const char* envarReleaseDir = getenv("BELLE2_RELEASE_DIR");
  const char* envarLocalDir = getenv("BELLE2_LOCAL_DIR");
  const char* envarAnalysisDir = getenv("BELLE2_ANALYSIS_DIR");
  if (!envarReleaseDir and !envarLocalDir) {
    B2FATAL("The basf2 environment is not set up. Please execute the 'setuprel' script first.");
  }

  //also set when just sourcing setup_belle2.sh (which is why we also check for local/release dir)
  const char* envarSubDir = getenv("BELLE2_SUBDIR");
  if (!envarSubDir) {
    B2FATAL("The environment variable BELLE2_SUBDIR is not set. Please execute the 'setuprel' script first.");
  }

  const char* envarExtDir = getenv("BELLE2_EXTERNALS_DIR");
  if (!envarExtDir) {
    B2FATAL("The environment variable BELLE2_EXTERNALS_DIR is not set. Please execute the 'setuprel' script first.");
  }

  // add module directories for current build options
  std::string added_dirs;
  if (envarAnalysisDir) {
    const string analysisModules = (fs::path(envarAnalysisDir) / "modules" / envarSubDir).string();
    ModuleManager::Instance().addModuleSearchPath(analysisModules);
    added_dirs += analysisModules;
  }

  if (envarLocalDir) {
    const string localModules = (fs::path(envarLocalDir) / "modules" / envarSubDir).string();
    ModuleManager::Instance().addModuleSearchPath(localModules);
    added_dirs += " " + localModules;
  }

  if (envarReleaseDir) {
    const string centralModules = (fs::path(envarReleaseDir) / "modules" / envarSubDir).string();
    ModuleManager::Instance().addModuleSearchPath(centralModules);
    added_dirs += " " + centralModules;
  }

  if (ModuleManager::Instance().getAvailableModules().empty()) {
    B2ERROR("No modules found! Did you forget to run 'scons'? Module paths added: " << added_dirs);
  }

  //set path to external software
  setExternalsPath(envarExtDir);
}


Environment::~Environment()
{
}

void Environment::setJobInformation(const std::shared_ptr<Path>& path)
{
  const std::list<ModulePtr>& modules = path->getModules();

  for (ModulePtr m : modules) {
    string type = m->getType();
    if (type == "RootInput") {
      bool ignoreOverride = m->getParam<bool>("ignoreCommandLineOverride").getValue();
      std::vector<std::string> inputFiles = Environment::Instance().getInputFilesOverride();
      if (ignoreOverride or inputFiles.empty()) {
        string inputFileName = m->getParam<string>("inputFileName").getValue();
        if (!inputFileName.empty())
          inputFiles.push_back(inputFileName);
        else
          inputFiles = m->getParam<vector<string> >("inputFileNames").getValue();
      }

      for (string file : inputFiles) {
        m_jobInfoOutput += "INPUT FILE: " + file + "\n";
      }
    } else if (type == "RootOutput") {
      std::string out = Environment::Instance().getOutputFileOverride();
      bool ignoreOverride = m->getParam<bool>("ignoreCommandLineOverride").getValue();
      if (ignoreOverride or out.empty())
        out = m->getParam<string>("outputFileName").getValue();
      m_jobInfoOutput += "OUTPUT FILE: " + out + "\n";
    }
  }
}

void Environment::printJobInformation() const
{
  cout << m_jobInfoOutput;
}
