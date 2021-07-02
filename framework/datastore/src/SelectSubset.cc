/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/datastore/SelectSubset.h>

#include <framework/datastore/RelationArray.h>

using namespace Belle2;

void SelectSubsetBase::swapSetsAndDestroyOriginal()
{
  StoreAccessorBase* set = getSet();
  StoreAccessorBase* subset = getSubSet();

  //replace set with subset
  DataStore::Instance().replaceData(*subset, *set);

  //swap relations
  for (const std::string& fromArray : m_inheritFromArrays) {
    RelationArray setRel(DataStore::relationName(fromArray, set->getName()));
    RelationArray subsetRel(DataStore::relationName(fromArray, subset->getName()));
    DataStore::Instance().replaceData(subsetRel, setRel);
  }
  for (const std::string& toArray : m_inheritToArrays) {
    RelationArray setRel(DataStore::relationName(set->getName(), toArray));
    RelationArray subsetRel(DataStore::relationName(subset->getName(), toArray));
    DataStore::Instance().replaceData(subsetRel, setRel);
  }
}
