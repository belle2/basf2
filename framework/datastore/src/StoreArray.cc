/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreArray.h>

#include <framework/datastore/RelationArray.h>

using namespace std;
using namespace Belle2;

void _StoreArrayImpl::clearRelations(const StoreAccessorBase& array)
{
  auto related_arrays = DataStore::Instance().getListOfRelatedArrays(array);

  for (const std::string& other_name : related_arrays) {
    RelationArray a(DataStore::relationName(other_name, array.getName()), array.getDurability());
    RelationArray b(DataStore::relationName(array.getName(), other_name), array.getDurability());
    if (a.isValid())
      a.clear();
    if (b.isValid())
      b.clear();
  }
}
