/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pybasf2/PyStoreObj.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>
#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TClass.h>

using namespace Belle2;
using namespace std;

namespace {
  template<class T>
  T* replaceNullPtr(T* value, T* fallback)
  {
    return value ? value : fallback;
  }
}

vector<string> PyStoreObj::list(DataStore::EDurability durability)
{
  return DataStore::Instance().getListOfObjects(TObject::Class(), durability);
}

void PyStoreObj::printList(DataStore::EDurability durability)
{
  for (const auto& n : list(durability))
    B2INFO(n);
}


PyStoreObj::PyStoreObj(const std::string& name,
                       DataStore::EDurability durability):
  PyStoreObj(replaceNullPtr(DataStore::getTClassFromDefaultObjectName(name),
                            TObject::Class()),
             /* Default to TObject for unknown class for backwards compatability */
             name,
             durability)
{
}

PyStoreObj::PyStoreObj(TClass* objClass,
                       DataStore::EDurability durability) :
  PyStoreObj(objClass, DataStore::defaultObjectName(objClass), durability)
{
}

PyStoreObj::PyStoreObj(TClass* objClass,
                       const std::string& name,
                       DataStore::EDurability durability) :
  m_storeAccessor(name, durability, objClass, false)
{
  // Attach if already created
  attach();
}

bool PyStoreObj::registerInDataStore(DataStore::EStoreFlags storeFlags)
{
  return registerInDataStore(m_storeAccessor.getName(), storeFlags);
}

bool PyStoreObj::registerInDataStore(const std::string& name, DataStore::EStoreFlags storeFlags)
{
  if (not hasValidClass()) {
    B2ERROR("Cannot register PyStoreObj '" << name << "' with unknown TClass. Please supply one to the PyStoreObj constructor.");
    return false;
  }

  bool success = m_storeAccessor.registerInDataStore(name, storeFlags);
  if (success) attach();
  return success;
}

bool PyStoreObj::isRequired(const std::string& name)
{
  return m_storeAccessor.isRequired(name);
}

bool PyStoreObj::isOptional(const std::string& name)
{
  return m_storeAccessor.isOptional(name);
}

bool PyStoreObj::isValid() const
{
  return m_storeEntry and m_storeEntry->ptr;
}

bool PyStoreObj::hasValidClass() const
{
  const TClass* objClass = m_storeAccessor.getClass();
  return objClass and objClass != TObject::Class();
}

bool PyStoreObj::create(bool replace)
{
  ensureAttached();
  if (not m_storeEntry) {
    // Attaching failed
    B2ERROR("Cannot create unregistered PyStoreObj.");
    return false;
  }

  // Short cut when an object has been created and no replacement is requested.
  if (isValid() and not replace) return true;

  if (not isValid() and not hasValidClass()) {
    B2ERROR("Cannot create PyStoreObj with unknown TClass.");
    return false;
  } else {
    // StoreObj has been created before or has a valid class
    // Go ahead and (re)create it
    return m_storeAccessor.create(replace);
  }
}

void PyStoreObj::ensureAttached() const
{
  if (not m_storeEntry) {
    attach();
  }
  if (not m_storeEntry) {
    B2ERROR("PyStoreObj " << m_storeAccessor.readableName() << " has not been registered!");
  }
}

void PyStoreObj::attach() const
{
  m_storeEntry = DataStore::Instance().getEntry(m_storeAccessor);
}

bool PyStoreObj::assign(TObject* object, bool replace)
{
  return m_storeAccessor.assign(object, replace);
}
