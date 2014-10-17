#include <framework/datastore/SelectSubset.h>

#include <framework/datastore/RelationArray.h>

using namespace Belle2;

void SelectSubsetBase::swapSetsAndDestroyOriginal()
{
  const StoreAccessorBase* set = getSet();
  StoreAccessorBase* subset = getSubSet();

  //replace set with subset
  DataStore::Instance().swap(*set, *subset);
  subset->clear();

  //swap relations
  for (std::string fromArray : m_inheritFromArrays) {
    RelationArray a(DataStore::relationName(fromArray, set->getName()));
    RelationArray b(DataStore::relationName(fromArray, subset->getName()));
    DataStore::Instance().swap(a, b);
    b.clear();
  }
  for (std::string toArray : m_inheritToArrays) {
    RelationArray a(DataStore::relationName(set->getName(), toArray));
    RelationArray b(DataStore::relationName(subset->getName(), toArray));
    DataStore::Instance().swap(a, b);
    b.clear();
  }
}
