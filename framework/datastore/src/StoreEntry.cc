#include <framework/datastore/StoreEntry.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/logging/Logger.h>

#include <TClass.h>
#include <TClonesArray.h>

using namespace Belle2;

StoreEntry::StoreEntry(bool isArray_, TClass* cl, const std::string& name_, bool dontWriteOut_):
  isArray(isArray_),
  dontWriteOut(dontWriteOut_),
  objClass(cl),
  object(nullptr),
  ptr(nullptr),
  name(name_)
{
  recoverFromNullObject();
}

void StoreEntry::recoverFromNullObject()
{
  if (object)
    return;
  if (isArray) {
    object = new TClonesArray(objClass);
  } else {
    // Oh dear, where to begin. So we want to create a new object of the class
    // we have and we require this class to be inheriting from TObject. Fine,
    // but there could be classes with multiple inheritance where the TObject
    // is not the first base class. In this case the memory layout puts the
    // TObject not at the beginning of the instance but at an offset. The
    // compiler knows this so a static_cast<> or c-style cast from one to the
    // other will correctly modify the pointing address to point to the start
    // of TObject, but TClass::New() gives us a void* pointer so the compiler
    // doesn't know about that. So to be on the safe side we have to manually
    // fix the pointer address using the BaseClassOffset from TClass. And since
    // pointer arithmetic on void* is forbidden we have to go to char* first.
    char* rawPtr = reinterpret_cast<char*>(objClass->New());
    int offset = objClass->GetBaseClassOffset(TObject::Class());
    if (offset < 0) B2FATAL("Class " << objClass->GetName() << " does not inherit from TObject");
    object = reinterpret_cast<TObject*>(rawPtr + offset);
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
  resetForGetEntry();
  if (object == nullptr)
    recoverFromNullObject();

  ptr = object;
}

void StoreEntry::invalidate()
{
  recreate();
  ptr = nullptr;
}

TClonesArray* StoreEntry::getPtrAsArray() const
{
  if (!isArray)
    return nullptr;
  return static_cast<TClonesArray*>(ptr);
}
