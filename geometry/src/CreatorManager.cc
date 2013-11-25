/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/CreatorManager.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <memory>

using namespace std;
using namespace Belle2;
using namespace Belle2::geometry;

CreatorManager& CreatorManager::getInstance()
{
  static auto_ptr<CreatorManager> instance(new CreatorManager());
  return *instance;
}

//map<string, CreatorManager::CreatorFactory*> CreatorManager::m_creatorFactories;

void CreatorManager::registerCreatorFactory(const std::string& name, CreatorFactory* factory)
{
  getInstance().m_creatorFactories[name] = factory;
}

CreatorBase* CreatorManager::getCreator(const string& name, const string& library)
{
  if (!library.empty()) {
    FileSystem::loadLibrary(library, false);
  }

  CreatorManager& instance = getInstance();
  map<string, CreatorFactory*>::const_iterator it = instance.m_creatorFactories.find(name);
  if (it == instance.m_creatorFactories.end()) {
    B2ERROR("Could not find a geometry creator named " << name);
    return 0;
  }
  return it->second();
}
