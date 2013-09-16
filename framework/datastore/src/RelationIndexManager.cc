/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#include <framework/datastore/RelationIndexManager.h>

using namespace Belle2;

RelationIndexManager& RelationIndexManager::Instance()
{
  static RelationIndexManager instance;
  return instance;
}
void RelationIndexManager::clear(DataStore::EDurability durability)
{
  RelationMap& relations = m_cache[durability];
  RelationMap::iterator end(relations.end());
  for (RelationMap::iterator it = relations.begin(); it != end; ++it)
    delete it->second;
  relations.clear();
}
