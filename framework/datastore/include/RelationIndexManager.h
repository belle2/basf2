/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationIndexContainer.h>

#include <array>
#include <memory>
#include <map>

namespace Belle2 {

  /** Manager to keep a cache of existing RelationIndexContainers.
   *
   *  This singleton keeps track of all RelationIndexContainers that where created
   *  to make sure indices are not created more often than needed.
   *
   *  This class is only used internally, users should use RelationsObject/RelationsInterface to access/add relations.
   */
  class RelationIndexManager {
  public:
    /** Returns the singleton instance. */
    static RelationIndexManager& Instance();

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
    template<class FROM, class TO> std::shared_ptr<RelationIndexContainer<FROM, TO>> get(const RelationArray& relation)
    {
      const static bool doTypeCheck = (FROM::Class() != TObject::Class() or TO::Class() != TObject::Class());
      if (doTypeCheck)
        relation.isValid();

      const std::string& name = relation.getName();
      DataStore::EDurability durability = relation.getDurability();
      RelationMap& relations =  m_cache[durability];
      std::shared_ptr<RelationIndexContainer<FROM, TO>> indexContainer;
      RelationMap::iterator it = relations.find(name);
      if (it != relations.end()) {
        //if existing array is of wrong type, we'll overwrite the shared_ptr here, but the index will live on with any RelationIndex objects that use it.
        indexContainer = std::dynamic_pointer_cast<RelationIndexContainer<FROM, TO>>(it->second);
      }
      if (!indexContainer) {
        indexContainer.reset(new RelationIndexContainer<FROM, TO>(relation));
        relations[name] = indexContainer;
      } else {
        indexContainer->rebuild(false);
      }
      return indexContainer;
    }

    /** Clear the cache of RelationIndexContainers with the given
     *  durability.
     *
     *  Since we check for modification of the RelationArrays, this
     *  function is normally not needed.
     *
     *  @param durability Which cache to clear
     */
    void clear(DataStore::EDurability durability = DataStore::c_Event);

  protected:
    /** Constructor hidden. */
    RelationIndexManager() { }
    /** Same for copy-constructor. */
    RelationIndexManager(const RelationIndexManager&) { }

    /** if the index exists in the cache, it is returned; otherwise NULL.
     *
     * The index is not rebuilt, which makes this mostly useful to do minor changes to the index in DataStore::addRelation().
     */
    template<class FROM, class TO> std::shared_ptr<RelationIndexContainer<FROM, TO>> getIndexIfExists(const std::string& name,
        DataStore::EDurability durability) const
    {
      const RelationMap& relations =  m_cache[durability];
      RelationMap::const_iterator it = relations.find(name);
      if (it != relations.end()) {
        return std::dynamic_pointer_cast<RelationIndexContainer<FROM, TO>>(it->second);
      } else {
        return nullptr;
      }
    }

    /** Clean cache on exit. */
    ~RelationIndexManager()
    {
      for (int i = 0; i < DataStore::c_NDurabilityTypes; i++)
        clear((DataStore::EDurability)i);
    }

    /** Maptype to keep track of all Containers of one durability */
    typedef std::map<std::string, std::shared_ptr<RelationIndexBase>> RelationMap;
    /** Cachetype for all Containers */
    typedef std::array<RelationMap, DataStore::c_NDurabilityTypes> RelationCache;
    /** Cache for all Containers */
    RelationCache m_cache;

    /** only DataStore should be able to get non-const indices. */
    friend class DataStore;
  };

} // end namespace Belle2
