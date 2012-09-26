/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONSINTERFACE_H
#define RELATIONSINTERFACE_H

#include <vector>
#include <string>
#include <TObject.h>
#include <framework/datastore/RelationEntry.h>
#include <framework/datastore/DataStore.h>

namespace Belle2 {

  class RelationsInterface {
  public:
    RelationsInterface(): m_cacheDataStoreEntry(0), m_cacheArrayIndex(-1) {}
    bool addRelationTo(const TObject* object, double weight = 1);
    template <class T> RelationVector<T> getRelationsTo(std::string name = "") {
      return static_cast<RelationVector<T> >(DataStore::Instance().getRelationsTo(static_cast<TObject*>(this), m_cacheDataStoreEntry, m_cacheArrayIndex, T::GetClass(), name));
    }
    template <class T> RelationVector<T> getRelationsFrom(std::string name = "") {
      return static_cast<RelationVector<T> >(DataStore::Instance().getRelationsFrom(static_cast<TObject*>(this), m_cacheDataStoreEntry, m_cacheArrayIndex, T::GetClass(), name));
    }
    template <class T> RelationVector<T> getRelationsWith(std::string name = "") {
      return static_cast<RelationVector<T> >(DataStore::Instance().getRelationsWith(static_cast<TObject*>(this), m_cacheDataStoreEntry, m_cacheArrayIndex, T::GetClass(), name));
    }
  private:
    mutable DataStore::StoreEntry* m_cacheDataStoreEntry;
    mutable int                    m_cacheArrayIndex;
  };

}

#endif /* RELATIONSINTERFACE_H */
