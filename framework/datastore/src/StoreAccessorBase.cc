/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreAccessorBase.h>
#include <framework/logging/Logger.h>

#include <TClass.h>
#include <TClonesArray.h>
#include <TObject.h>

using namespace Belle2;

std::string StoreAccessorBase::readableName() const
{
  std::string str(isArray() ? "array" : "object");
  str += " '" + getName() + "' (durability: ";
  switch (getDurability()) {
    case DataStore::c_Event:
      str += "event";
      break;
    case DataStore::c_Persistent:
      str += "persistent";
      break;
  }
  return str + ")";
}

bool StoreAccessorBase::assign(TObject* object, bool replace)
{
  if (not object)
    return false;

  bool success = false;
  const bool objIsArray = (object->IsA() == TClonesArray::Class());
  TClass* objClass = objIsArray ? (static_cast<TClonesArray*>(object))->GetClass() : object->IsA();
  if (objIsArray != isArray()) {
    B2ERROR("Cannot assign an object to an array (or vice versa); while assigning to " << readableName());
  } else if (objClass != getClass()) {
    B2ERROR("Cannot assign() an object of type '" << objClass->GetName() << "' to " << readableName() << " of type '" <<
            getClass()->GetName() << "'!");
  } else {
    success = DataStore::Instance().createObject(object, replace, *this);
  }
  if (!success)
    delete object;
  return success;
}
bool StoreAccessorBase::notWrittenOut() const
{
  const DataStore::StoreEntry* entry = DataStore::Instance().getEntry(*this);
  if (!entry) {
    B2ERROR("notWrittenOut(): " << readableName() << " doesn't seem to  be registered");
    return false;
  }
  return entry->dontWriteOut;
}
