#include <framework/datastore/StoreEntry.h>
#include <framework/dataobjects/RelationContainer.h>

#include <TClass.h>
#include <TClonesArray.h>

using namespace Belle2;

void StoreEntry::resetForGetEntry()
{
  if (isArray) {
    static_cast<TClonesArray*>(object)->Delete();
  } else if (object->IsA() == RelationContainer::Class()) {
    static_cast<RelationContainer*>(object)->Clear();
  } else {
    //we don't know anything about object, so we just delete it here (and recreate later)
    delete object;
    object = nullptr;
  }
}

void StoreEntry::recreate()
{
  TClass* objClass = object->IsA();
  resetForGetEntry();
  if (object == nullptr)
    object = static_cast<TObject*>(objClass->New());

  ptr = object;
}
