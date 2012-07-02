/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef RelationIndexManager_H
#define RelationIndexManager_H

#include <boost/array.hpp>
#include <framework/datastore/RelationIndexContainer.h>

namespace Belle2 {

  /** Manager to keep a cache of existing RelationIndexContainers.
   *
   * This class keeps track of all RelationIndexContainers that where created
   * to make sure indices are not created more often than needed.
   *
   * It is a purely static class
   */
  class RelationIndexManager {
  public:

    /** Get a RelationIndexContainer.
     *
     *  Return a const reference to a RelationIndexContainer for the
     *  given name and durability. If none exists it will be created. If
     *  it already exists we rebuild the index if the underlying
     *  RelationArray has changed in the meantime
     *
     *  @param relation Relation to build an index for
     *  @returns A RelationIndexContainer
     */
    template<class FROM, class TO> static const RelationIndexContainer<FROM, TO> &get(const RelationArray& relation) {
      const std::string& name = relation.getName();
      DataStore::EDurability durability = relation.getDurability();
      RelationMap& relations =  m_cache[durability];
      RelationIndexContainer<FROM, TO> *indexContainer(0);
      RelationMap::iterator it = relations.find(name);
      if (it != relations.end()) {
        indexContainer = dynamic_cast< RelationIndexContainer<FROM, TO>* >(it->second);
      }
      if (!indexContainer) {
        indexContainer = new RelationIndexContainer<FROM, TO>(relation.getAccessorParams());
        relations[name] = indexContainer;
      } else {
        indexContainer->rebuild(false);
      }
      return *indexContainer;
    }

    /** Clear the cache of RelationIndexContainers with the given
     *  durability.
     *
     *  Since we check for modification of the RelationArrays, this
     *  function is normally not needed.
     *
     *  @param durability Which cache to clear
     */
    static void clear(DataStore::EDurability durability = DataStore::c_Event) {
      RelationMap& relations = m_cache[durability];
      RelationMap::iterator end(relations.end());
      for (RelationMap::iterator it = relations.begin(); it != end; ++it)
        delete it->second;
      relations.clear();
    }

  protected:
    /** Maptype to keep track of all Containers of one durability */
    typedef std::map<std::string, RelationIndexBase* > RelationMap;
    /** Cachetype for all Containers */
    typedef boost::array<RelationMap, DataStore::c_NDurabilityTypes> RelationCache;
    /** Cache for all Containers */
    static RelationCache m_cache;
  };

} // end namespace Belle2

#endif
