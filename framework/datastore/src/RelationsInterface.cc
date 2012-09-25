/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/RelationsInterface.h>

using namespace Belle2;

bool RelationsInterface::addRelationTo(const TObject* object, double weight)
{
  return DataStore::Instance().addRelation(reinterpret_cast<TObject*>(this), m_cacheDataStoreEntry, m_cacheArrayIndex, object, weight);
}

