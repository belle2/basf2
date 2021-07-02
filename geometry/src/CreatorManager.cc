/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  static unique_ptr<CreatorManager> instance(new CreatorManager());
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
  auto it = instance.m_creatorFactories.find(name);
  if (it == instance.m_creatorFactories.end()) {
    B2ERROR("Could not find a geometry creator named " << name);
    return nullptr;
  }
  return it->second();
}
