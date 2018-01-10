/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/core/ModuleManager.h>
#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <boost/filesystem.hpp>
#include <regex>

#include <fstream>

using namespace Belle2;
using namespace std;

#define MAP_FILE_EXTENSION ".map"
#define LIB_FILE_EXTENSION ".so"


ModuleManager& ModuleManager::Instance()
{
  static ModuleManager instance;
  return instance;
}


void ModuleManager::registerModuleProxy(ModuleProxyBase* moduleProxy)
{
  //Only register a module proxy if it was not yet registered.
  if (m_registeredProxyMap.count(moduleProxy->getModuleName()) == 0) {
    m_registeredProxyMap.insert(make_pair(moduleProxy->getModuleName(), moduleProxy));
  } else {
    B2ERROR("There seems to be more than one module called '" << moduleProxy->getModuleName() <<
            "'. Since module names are unique, you must rename one of them!");
  }
}


void ModuleManager::addModuleSearchPath(const string& path)
{
  if (FileSystem::isDir(path)) {
    m_moduleSearchPathList.push_back(path);

    //Search the path for map files and add the contained module names to the known module names
    auto fullPath = boost::filesystem::system_complete(boost::filesystem::path(path));
    boost::filesystem::directory_iterator endIter;


    map<string, string> moduleNameLibMap;
    for (boost::filesystem::directory_iterator dirItr(fullPath); dirItr != endIter; ++dirItr) {
      //Only files in the given folder are taken, subfolders are not used.
      if (boost::filesystem::is_regular_file(dirItr->status())) {
        if (boost::filesystem::extension(dirItr->path()) == MAP_FILE_EXTENSION) {
          fillModuleNameLibMap(moduleNameLibMap, *dirItr);
        }
      }
    }
    //put modules into central map, if they haven't been  added yet
    for (auto& entry : moduleNameLibMap) {
      if (m_moduleNameLibMap.count(entry.first) == 0) {
        m_moduleNameLibMap[entry.first] = entry.second;
      }
    }
  }
}


const list<string>& ModuleManager::getModuleSearchPaths() const
{
  return m_moduleSearchPathList;
}


const map<string, string>& ModuleManager::getAvailableModules() const
{
  return m_moduleNameLibMap;
}


ModulePtr ModuleManager::registerModule(const string& moduleName, const std::string& sharedLibPath) noexcept(false)
{
  map<string, ModuleProxyBase*>::iterator moduleIter =  m_registeredProxyMap.find(moduleName);

  //If the proxy of the module was not already registered, load the corresponding shared library first
  if (moduleIter == m_registeredProxyMap.end()) {

    //If a shared library path is given, load the library and search for the registered module.
    if (!sharedLibPath.empty()) {
      if (FileSystem::isFile(sharedLibPath)) {
        FileSystem::loadLibrary(sharedLibPath);
        moduleIter =  m_registeredProxyMap.find(moduleName);
      } else B2WARNING("Could not load shared library " + sharedLibPath + ". File does not exist!");

      //Check if the loaded shared library file contained the module
      if (moduleIter == m_registeredProxyMap.end()) {
        B2ERROR("The shared library " + sharedLibPath + " does not contain the module " + moduleName + "!");
      }
    } else {
      //If no library path is given, check if the module name is known to the manager and load
      //the appropriate shared library.
      map<string, string>::const_iterator libIter = m_moduleNameLibMap.find(moduleName);

      if (libIter != m_moduleNameLibMap.end()) {
        FileSystem::loadLibrary(libIter->second);
        moduleIter =  m_registeredProxyMap.find(moduleName);

        //Check if the loaded shared library file contained the module
        if (moduleIter == m_registeredProxyMap.end()) {
          B2ERROR("The shared library " + libIter->second + " does not contain the module " + moduleName + "!");
        }
      } else {
        B2ERROR("The module " + moduleName + " is not known to the framework!");
      }
    }
  }

  //Create an instance of the module found or loaded in the previous steps and return it.
  //If the iterator points to the end of the map, throw an exception
  if (moduleIter != m_registeredProxyMap.end()) {
    ModulePtr currModulePtr = moduleIter->second->createModule();
    m_createdModulesList.push_back(currModulePtr);
    return currModulePtr;
  } else throw (ModuleNotCreatedError() << moduleName);
}


const ModulePtrList& ModuleManager::getCreatedModules() const
{
  return m_createdModulesList;
}


ModulePtrList ModuleManager::getModulesByProperties(const ModulePtrList& modulePathList, unsigned int propertyFlags)
{
  ModulePtrList tmpModuleList;

  for (const ModulePtr& module : modulePathList)
    if (module->hasProperties(propertyFlags)) tmpModuleList.push_back(module);

  return tmpModuleList;
}

bool ModuleManager::allModulesHaveFlag(const ModulePtrList& list, unsigned int flag)
{
  for (auto m : list) {
    if (!m->hasProperties(flag))
      return false;
  }
  return true;
}


//============================================================================
//                              Private methods
//============================================================================

void ModuleManager::fillModuleNameLibMap(std::map<std::string, std::string>& moduleNameLibMap,
                                         const boost::filesystem::directory_entry& mapPath)
{
  //Check if the associated shared library file exists
  string sharedLibPath = boost::filesystem::change_extension(mapPath, LIB_FILE_EXTENSION).string();
  if (!FileSystem::fileExists(sharedLibPath)) {
    B2WARNING("The shared library file: " << sharedLibPath << " doesn't exist, but is required by " << mapPath.path().string());
    return;
  }

  //Open the map file and parse the content line by line
  ifstream mapFile(mapPath.path().string().c_str());
  string currentLine;

  //Read each line of the map file and use boost regular expression to find the module name string in brackets.
  string::const_iterator start, end;
  std::regex expression("\\((.+)\\)");
  std::match_results<std::string::const_iterator> matchResult;

  while (getline(mapFile, currentLine)) {
    start = currentLine.begin();
    end = currentLine.end();
    std::regex_search(start, end, matchResult, expression, std::regex_constants::match_default);

    //We expect exactly two result entries: [0] the string that matched the regular expression
    //                                      [1] the string that matched sub-expressions (here: the string inside the quotes)
    if (matchResult.size() == 2) {
      string moduleName(matchResult[1].first, matchResult[1].second);
      //Add result to map
      if (moduleNameLibMap.count(moduleName) == 0) {
        moduleNameLibMap.insert(make_pair(moduleName, sharedLibPath));
      } else {
        B2ERROR("There seems to be more than one module called '" << moduleName <<
                "'. Since module names are unique, you must rename one of them!");
      }
    } else {
      B2ERROR("Regular expression did not work. Is the module map file well formatted?");
    }
  }

  //Close the map file
  mapFile.close();
}

ModuleManager::ModuleManager()
{

}


ModuleManager::~ModuleManager()
{
}

void ModuleManager::reset()
{
  m_createdModulesList.clear();
}

