/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pybasf2/PyStoreArray.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

#include "TClonesArray.h"
#include "TClass.h"

using namespace Belle2;
using namespace std;

namespace {
  template<class T>
  T* replaceNullPtr(T* value, T* fallback)
  {
    return value ? value : fallback;
  }
}

std::vector<std::string> PyStoreArray::list(DataStore::EDurability durability)
{
  return DataStore::Instance().getListOfArrays(TObject::Class(),
                                               durability);
}

void PyStoreArray::printList(DataStore::EDurability durability)
{
  for (const auto& n : list(durability))
    B2INFO(n);
}

PyStoreArray::PyStoreArray(const std::string& name,
                           DataStore::EDurability durability):
  PyStoreArray(replaceNullPtr(DataStore::getTClassFromDefaultArrayName(name),
                              TObject::Class()),
               /* Default to TObject for unknown class for backwards compatability */
               name,
               durability)
{
}

PyStoreArray::PyStoreArray(TClass* objClass,
                           DataStore::EDurability durability) :
  PyStoreArray(objClass, DataStore::defaultArrayName(objClass), durability)
{
}

PyStoreArray::PyStoreArray(TClass* objClass,
                           const std::string& name,
                           DataStore::EDurability durability) :
  m_storeAccessor(name, durability, objClass, true)
{
  attach();
}

bool PyStoreArray::registerInDataStore(DataStore::EStoreFlags storeFlags)
{
  return registerInDataStore(m_storeAccessor.getName(), storeFlags);
}

bool PyStoreArray::registerInDataStore(const std::string& name,
                                       DataStore::EStoreFlags storeFlags)
{
  if (not hasValidClass()) {
    B2ERROR("Cannot register PyStoreArray '" << name << "' with unknown TClass. Please supply one to the PyStoreArray constructor.");
    return false;
  }

  bool success = m_storeAccessor.registerInDataStore(name, storeFlags);
  if (success) attach();
  return success;
}

bool PyStoreArray::isRequired(const std::string& name)
{
  return m_storeAccessor.isRequired(name);
}

bool PyStoreArray::isOptional(const std::string& name)
{
  return m_storeAccessor.isOptional(name);
}

bool PyStoreArray::registerRelationTo(const PyStoreArray& toArray,
                                      DataStore::EDurability durability,
                                      DataStore::EStoreFlags storeFlags,
                                      std::string const& namedRelation) const
{
  return DataStore::Instance().registerRelation(this->m_storeAccessor,
                                                toArray.m_storeAccessor,
                                                durability,
                                                storeFlags,
                                                namedRelation);
}

bool PyStoreArray::requireRelationTo(const PyStoreArray& toArray,
                                     DataStore::EDurability durability,
                                     std::string const& namedRelation) const
{
  return DataStore::Instance().requireRelation(this->m_storeAccessor,
                                               toArray.m_storeAccessor,
                                               durability,
                                               namedRelation);
}

bool PyStoreArray::optionalRelationTo(const PyStoreArray& toArray,
                                      DataStore::EDurability durability,
                                      std::string const& namedRelation) const
{
  return DataStore::Instance().requireRelation(this->m_storeAccessor,
                                               toArray.m_storeAccessor,
                                               durability,
                                               namedRelation);
}

bool PyStoreArray::hasRelationTo(const PyStoreArray& toArray,
                                 DataStore::EDurability durability,
                                 const std::string& namedRelation) const
{
  return DataStore::Instance().hasRelation(this->m_storeAccessor,
                                           toArray.m_storeAccessor,
                                           durability,
                                           namedRelation);
}

bool PyStoreArray::hasRelationFrom(const PyStoreArray& fromArray,
                                   DataStore::EDurability durability,
                                   const std::string& namedRelation) const
{
  return DataStore::Instance().hasRelation(fromArray.m_storeAccessor,
                                           this->m_storeAccessor,
                                           durability,
                                           namedRelation);
}

bool PyStoreArray::hasValidClass() const
{
  const TClass* objClass = m_storeAccessor.getClass();
  return objClass and objClass != TObject::Class();
}

bool PyStoreArray::isValid() const
{
  return m_storeEntry and m_storeEntry->ptr;
}

TObject* PyStoreArray::operator [](int i) const
{
  ensureAttached();
  if (not isValid()) {
    return nullptr;
  } else {
    return m_storeEntry->getPtrAsArray()->At(i);
  }
}

int PyStoreArray::getEntries() const
{
  ensureAttached();
  return isValid() ? (m_storeEntry->getPtrAsArray()->GetEntriesFast()) : 0;
}

TIter PyStoreArray::__iter__() const
{
  ensureAttached();
  // will create empty iterator if nullptr
  return TIter(isValid() ? m_storeEntry->getPtrAsArray() : nullptr);
}

TObject* PyStoreArray::appendNew()
{
  ensureCreated();
  if (isValid()) {
    return getPtr()->ConstructedAt(getEntries());
  } else {
    B2ERROR("Cannot create an object in invalid PyStoreArray.");
    return nullptr;
  }
}

TClonesArray* PyStoreArray::getPtr()
{
  ensureCreated();
  return isValid() ? m_storeEntry->getPtrAsArray() : nullptr;
}

void PyStoreArray::ensureCreated()
{
  create(false);
}

bool PyStoreArray::create(bool replace)
{
  ensureAttached();
  if (not m_storeEntry) {
    // Attaching failed
    B2ERROR("Cannot create unregistered PyStoreArray.");
    return false;
  }

  // Short cut when an array has been created and no replacement is requested.
  if (isValid() and not replace) return true;

  if (not isValid() and not hasValidClass()) {
    B2ERROR("Cannot create PyStoreArray with unknown TClass.");
    return false;
  } else {
    // Array has been created before or has a valid class
    // Go ahead and (re)create it
    return m_storeAccessor.create(replace);
  }
}

void PyStoreArray::ensureAttached() const
{
  if (not m_storeEntry) {
    attach();
  }
  if (not m_storeEntry) {
    B2ERROR("PyStoreArray " << m_storeAccessor.readableName() << " has not been registered!");
  }
}

void PyStoreArray::attach() const
{
  m_storeEntry = DataStore::Instance().getEntry(m_storeAccessor);
}
