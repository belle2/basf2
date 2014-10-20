#include <framework/datastore/SelectSubset.h>

#include <framework/datastore/RelationArray.h>

using namespace Belle2;

void SelectSubsetBase::swapSetsAndDestroyOriginal()
{
  StoreAccessorBase* set = getSet();
  StoreAccessorBase* subset = getSubSet();

  //replace set with subset
  set->clear();
  DataStore::Instance().swap(*set, *subset);

  //swap relations
  for (std::string fromArray : m_inheritFromArrays) {
    RelationArray a(DataStore::relationName(fromArray, set->getName()));
    RelationArray b(DataStore::relationName(fromArray, subset->getName()));
    a.clear();
    DataStore::Instance().swap(a, b);
  }
  for (std::string toArray : m_inheritToArrays) {
    RelationArray a(DataStore::relationName(set->getName(), toArray));
    RelationArray b(DataStore::relationName(subset->getName(), toArray));
    a.clear();
    DataStore::Instance().swap(a, b);
  }
}
