/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorManager.h>
#include <geometry/geodetector/CreatorBase.h>

using namespace std;
using namespace Belle2;


CreatorManager* CreatorManager::m_instance = NULL;


CreatorManager& CreatorManager::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new CreatorManager();
  return *m_instance;
}


void CreatorManager::registerCreator(CreatorBase* creator, bool force) throw(GeometryCreatorNameEmptyError, GeometryCreatorAlreadyExistsError)
{
  //Check if the name of the creator is empty
  if (creator->getName().empty()) throw GeometryCreatorNameEmptyError();

  //Check if a creator with the given name already exists
  map<string, CreatorBase*>::iterator mapIter = m_creatorMap.find(creator->getName());

  if (mapIter != m_creatorMap.end()) {
    if (!force) throw GeometryCreatorAlreadyExistsError(creator->getName());
    else {
      delete mapIter->second;
      m_creatorMap.erase(mapIter);
    }
  }

  m_creatorMap.insert(make_pair(creator->getName(), creator));
}


CreatorBase& CreatorManager::getCreator(const std::string& name) const throw(GeometryCreatorNameEmptyError, GeometryCreatorNotExistsError)
{
  //Check if the name of the creator is empty
  if (name.empty()) throw GeometryCreatorNameEmptyError();

  map<string, CreatorBase*>::const_iterator mapIter = m_creatorMap.find(name);

  if (mapIter == m_creatorMap.end()) throw GeometryCreatorNotExistsError(name);
  return *mapIter->second;
}


list<string> CreatorManager::getCreatorList() const
{
  list<string> returnList;
  map<string, CreatorBase*>::const_iterator mapIter;

  for (mapIter = m_creatorMap.begin(); mapIter != m_creatorMap.end(); mapIter++) {
    returnList.push_back(mapIter->first);
  }
  return returnList;
}


//============================================================================
//                              Private methods
//============================================================================

CreatorManager::CreatorManager()
{

}


CreatorManager::~CreatorManager()
{

}
