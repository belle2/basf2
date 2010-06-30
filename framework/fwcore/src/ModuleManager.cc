/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <fwcore/ModuleManager.h>

#include <dlfcn.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include <logging/Logger.h>

using namespace Belle2;
using namespace std;

ModuleManager* ModuleManager::m_instance = NULL;


ModuleManager& ModuleManager::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new ModuleManager();
  return *m_instance;
}


void ModuleManager::loadModuleLibs(const string& path)
{
  if (path.empty()) return;

  boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());

  try {
    fullPath = boost::filesystem::system_complete(boost::filesystem::path(path));
    if (!boost::filesystem::exists(fullPath)) {
      WARNING("Could not load module library ! Invalid path: " + path);
      return;
    }

    //If the path is a directory, search for libraries in the directory
    //if not try to load the given file as a library.
    if (boost::filesystem::is_directory(fullPath)) {
      boost::filesystem::directory_iterator endIter;
      for (boost::filesystem::directory_iterator dirItr(fullPath); dirItr != endIter; ++dirItr) {

        //Only files in the given folder are taken, subfolders are not used.
        if (boost::filesystem::is_regular(dirItr->status())) {
          //Take only files having the correct module extension
          if (isExtensionSupported(boost::filesystem::extension(dirItr->path()))) {
            loadLibrary(dirItr->path().filename(), dirItr->path().string());
          }
        }
      }
    } else {
      if (isExtensionSupported(boost::filesystem::extension(fullPath))) {
        loadLibrary(fullPath.filename(), fullPath.string());
      }
    }
  } catch (...) {
    ERROR("Could not load module library ! Invalid path: " + path);
  }
}


void ModuleManager::registerModule(Module* module)
{
  //Only do module self-registration if the module does not yet exist.
  if (m_typeModuleMap.find(module->getType()) == m_typeModuleMap.end()) {
    m_typeModuleMap.insert(make_pair(module->getType(), module));
  } else {
    ERROR("Could not self-register Module '" + module->getType() + "' ! A module of this type already exists.");
  }
}


const Module& ModuleManager::getModuleByType(const string& type) const throw(FwExcModuleTypeNotFound)
{
  map<const string, Module*>::const_iterator findIter = m_typeModuleMap.find(type);

  if (findIter == m_typeModuleMap.end()) {
    throw FwExcModuleTypeNotFound(type);
  } else return *(findIter->second);
}


list<ModulePtr> ModuleManager::getAvailableModules() const
{
  list<ModulePtr> returnList;

  map<const string, Module*>::const_iterator mapIter;
  for (mapIter = m_typeModuleMap.begin(); mapIter != m_typeModuleMap.end(); mapIter++) {
    ModulePtr returnModule = mapIter->second->newModule();
    returnList.push_back(returnModule);
  }

  return returnList;
}


ModulePtr ModuleManager::createModule(const std::string& type) const throw(FwExcModuleNotCreated)
{
  try {
    Module& newModule = const_cast<Module&>(getModuleByType(type));
    return newModule.newModule();
  } catch (FwExcModuleTypeNotFound& exc) {

    ERROR("Could not create a module of type '" + exc.getModuleType() + "' ! Module type was not found.");
    throw FwExcModuleNotCreated(type);
  }
}


void ModuleManager::addLibFileExtension(const std::string& fileExt)
{
  m_libFileExtensions.insert(fileExt);
}


//============================================================================
//                              Private methods
//============================================================================

void ModuleManager::loadLibrary(const std::string& libName, const std::string& libPath)
{
  //Check if library with same name was not already added
  if (m_nameLibraryMap.find(libName) != m_nameLibraryMap.end()) {
    ERROR("Could not load module library '" + libName + "' ! A module library with the same name was already loaded.");
    return;
  }

  unsigned int numRegModBefore = m_nameLibraryMap.size();

  //Open the library. By opening the library, the Modules register themselves.
  void* libPointer = dlopen(libPath.c_str() , RTLD_LAZY | RTLD_GLOBAL);

  if (libPointer == NULL) {
    ERROR("Could not open shared library file (error in dlopen) : " + string(dlerror()));
  } else {
    //Check if modules were registered, otherwise close the library
    if (m_typeModuleMap.size() > numRegModBefore) {
      m_nameLibraryMap.insert(make_pair(libName, libPointer));
    } else {
      dlclose(libPointer);
    }
  }
}


void ModuleManager::closeOpenLibraries()
{
  map<const string, void*>::iterator mapIter;

  for (mapIter = m_nameLibraryMap.begin(); mapIter != m_nameLibraryMap.end(); mapIter++) {
    dlclose(mapIter->second);
  }
}


bool ModuleManager::isExtensionSupported(const std::string& fileExt) const
{
  return m_libFileExtensions.find(fileExt) != m_libFileExtensions.end();
}


ModuleManager::ModuleManager()
{

}


ModuleManager::~ModuleManager()
{
  closeOpenLibraries();
}
