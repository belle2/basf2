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
