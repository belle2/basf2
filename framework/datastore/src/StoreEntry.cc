#include <framework/datastore/StoreEntry.h>
#include <framework/dataobjects/RelationContainer.h>

#include <TClass.h>
#include <TClonesArray.h>

using namespace Belle2;

StoreEntry::StoreEntry(bool isArray, const TClass* cl, const std::string& name, bool dontWriteOut):
  isArray(isArray),
  dontWriteOut(dontWriteOut),
  objClass(cl),
  ptr(nullptr),
  name(name)
{
  if (isArray) {
    object = new TClonesArray(objClass);
  } else {
    object = static_cast<TObject*>(objClass->New());
  }
}

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

TClonesArray* StoreEntry::getPtrAsArray() const
{
  if (!isArray)
    return nullptr;
  return static_cast<TClonesArray*>(ptr);
}
