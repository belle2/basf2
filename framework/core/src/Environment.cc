/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>
#include <framework/logging/LogConfig.h>
#include <framework/core/InputController.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/FileMetaData.h>

#include <boost/filesystem.hpp>
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

bool Environment::isMC() const
{
  StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
  if (fileMetaData) return fileMetaData->isMC();
  return true;
}

void Environment::setRealm(LogConfig::ELogRealm realm)
{
  m_realm = realm;
  B2INFO("Execution realm: " << LogConfig::logRealmToString(realm));
}

std::string Environment::consumeOutputFileOverride(const std::string& module)
{
  std::string s{""};
  if (!m_outputFileOverrideModule.empty()) {
    B2WARNING("Module '" << module << "' requested to handle -o which has already been handled by '" << module << "', ignoring");
    return s;
  }
  if (!m_outputFileOverride.empty()) {
    m_outputFileOverrideModule = module;
    std::swap(s, m_outputFileOverride);
  }
  return s;
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

  // add module directories for current build options, starting with the working directory on program startup
  std::string added_dirs = fs::initial_path().string();
  ModuleManager::Instance().addModuleSearchPath(added_dirs);

  if (envarAnalysisDir) {
    const string analysisModules = (fs::path(envarAnalysisDir) / "modules" / envarSubDir).string();
    ModuleManager::Instance().addModuleSearchPath(analysisModules);
    added_dirs += " " + analysisModules;
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

Environment::~Environment() = default;


// we know getFileNames is deprecated but we need it as long as --dry-run is available
// so let's remove the warning for now ...
#ifdef __INTEL_COMPILER
#pragma warning (disable:1478) //[[deprecated]]
#pragma warning (disable:1786) //[[deprecated("message")]]
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

void Environment::setJobInformation(const std::shared_ptr<Path>& path)
{
  const std::list<ModulePtr>& modules = path->buildModulePathList(true);

  for (const ModulePtr& m : modules) {
    if (m->hasProperties(Module::c_Input)) {
      std::vector<std::string> inputFiles = m->getFileNames(false);
      for (const string& file : inputFiles) {
        m_jobInfoOutput += "INPUT FILE: " + file + "\n";
      }
    }
    if (m->hasProperties(Module::c_Output)) {
      std::vector<std::string> outputFiles = m->getFileNames(true);
      for (const string& file : outputFiles) {
        m_jobInfoOutput += "OUTPUT FILE: " + file + "\n";
      }
    }
  }
}

void Environment::printJobInformation() const
{
  cout << m_jobInfoOutput;
}
