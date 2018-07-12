/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2017  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pybasf2/PyDBArray.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/DataStore.h>

#include <TClonesArray.h>
#include <TClass.h>

using namespace Belle2;

namespace {
  /** small helper to get TClass from the given payload name or default back
   * to TObject if no such class can be found */
  TClass* getDefaultClass(const std::string& name)
  {
    // First look for an name without the namespace Belle2::
    // For arrays the default name is the class name + 's' so lets strip it.
    TClass* cl = TClass::GetClass(("Belle2::" + name.substr(0, name.size() - 1)).c_str());
    if (!cl) {
      // If this fails look for a name that already has the full namespace.
      cl = TClass::GetClass(name.c_str());
    }
    if (!cl) return TObject::Class();
    return cl;
  }
}

const TObject* PyDBArray::_get(int i) const
{
  return getObject<TClonesArray>()->At(i);
}

int PyDBArray::getEntries() const
{
  return isValid() ? (getObject<TClonesArray>()->GetEntriesFast()) : 0;
}

PyDBArray::PyDBArray(const std::string& name, bool required): PyDBArray(name, getDefaultClass(name), required) {}

PyDBArray::PyDBArray(const TClass* objClass, bool required): PyDBArray(DataStore::defaultArrayName(objClass), objClass, required) {}

PyDBArray::PyDBArray(const std::string& name, const TClass* objClass, bool required): DBAccessorBase(name, objClass, true,
      required) {}
