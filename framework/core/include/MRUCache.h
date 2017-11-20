/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

namespace Belle2 {
  /**
   * Class implementing a generic Most Recently Used cache
   *
   * It can be used to cache values which are needed repeatedly. The size is
   * limited so only the N most recently accessed elements will be kept.
   *
   * Usage: if you have elements of type VALUE which are uniquely identified by
   * something of type KEY and you want to cache the last 100 of them because
   * computation is expensive, than you can define a cache somewhere:
   *
   * MRUCache<KEY, VALUE> cache(100);
   *
   * and in your function you can do
   *
   * VALUE calculate(const KEY &key){
   *    VALUE value;
   *    if(!cache.retrieve(key,value)){
   *        //calculate value here
   *        //and finally add to cache
   *        cache.insert(key,value);
   *    }
   *    return value;
   * }
   *
   * @tparam KEY Key type to identify entries of the cache
   * @tparam VALUE Value type of the cache entries
   */
  template <class KEY, class VALUE> class MRUCache {
  public:
    /** type of elements stored in the cache */
    typedef std::pair<KEY, VALUE> value_type;
    /** type of container for the elements */
    typedef boost::multi_index_container <
    value_type,
    boost::multi_index::indexed_by <
    boost::multi_index::sequenced<>,
    boost::multi_index::hashed_unique <
    boost::multi_index::member<value_type, KEY, &value_type::first>
    >
    >
    > container_type;
    /** iterator over all cached items, sorted by access: most recent used first */
    typedef typename container_type::const_iterator iterator;
    /** iterator over the hash index to the items */
    typedef typename container_type::template nth_index<1>::type::const_iterator hash_iterator;

    /**
     * Constructor setting the maximum number of cached items
     * @param maxSize Maximum number of cached Items
     */
    explicit MRUCache(size_t maxSize): m_maxSize(maxSize), m_hits(0), m_misses(0), m_overflows(0) {}

    /**
     * Insert a key value pair into the cache. If the maximum size is
     * reached, the least recently used item is dropped
     * @param key key to the new item
     * @param value to the new item
     */
    void insert(const KEY& key, const VALUE& value) { insert(value_type(key, value)); }

    /**
     * Insert a key value pair into the cache. If the maximum size is
     * reached, the least recently used item is dropped
     * @param item std::pair containing key and value of the new item
     */
    void insert(const value_type& item)
    {
      std::pair<iterator, bool> p = m_container.push_front(item);
      if (!p.second) {  /* duplicate item, put existing in front */
        m_container.replace(p.first, item);
        update(p.first);
      } else if (m_container.size() > m_maxSize) {  /* keep the length <= maxSize */
        ++m_overflows;
        m_container.pop_back();
      }
    }

    /**
     * Retrieve a value from the cache if it exists
     * @param[in] key    key for the value to retrieve
     * @param[out] value reference to the value. Will only be modified if an item is found
     * @return true if value could be found, false otherwise
     */
    bool retrieve(const KEY& key, VALUE& value)
    {
      hash_iterator it = m_container.template get<1>().find(key);
      if (it == m_container.template get<1>().end()) {
        ++m_misses;
        return false;
      }
      update(m_container.template project<0>(it));
      value = it->second;
      ++m_hits;
      return true;
    }

    /** Return iterator to the begin of the cache. Items are sorted by access: most recently inserted */
    iterator begin() const { return m_container.begin();  }
    /** Return iterator to the end of the cache */
    iterator end() const { return m_container.end();  }

    /** Return actual size of the cache */
    size_t size() const { return m_container.size(); }
    /** Clear cache */
    void clear()
    {
      m_container.clear();
      m_hits = 0;
      m_misses = 0;
      m_overflows = 0;
    }

    /** Set maximum number of cache entries */
    void setMaxSize(size_t maxSize) { m_maxSize = maxSize; }
    /** Get maximum number of cache entries */
    size_t getMaxSize() const { return m_maxSize; }
    /** Get number of cache hits since creation/last clear */
    unsigned int getHits() const { return m_hits; }
    /** Get number of cache misses since creation/last clear */
    unsigned int getMisses() const { return m_misses; }
    /** Get number of overflows (dropped items) since creation/last clear */
    unsigned int getOverflows() const { return m_overflows; }

  protected:
    /**
     * Update an item, thus marking it as recently accessed and putting it to
     * the front of the list
     * @param item iterator to the item to update
     */
    void update(const iterator& item)
    {
      m_container.relocate(m_container.begin(), item);
    }

    /** Container for the items */
    container_type m_container;
    /** Maximum size of the cache */
    size_t m_maxSize;
    /** Number of hits */
    unsigned int m_hits;
    /** Number of misses */
    unsigned int m_misses;
    /** Number of overflows */
    unsigned int m_overflows;
  };
}
