/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
