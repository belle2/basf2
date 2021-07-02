/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  for (auto& e : relations) {
    if (e.second) e.second->clear();
  }
}
