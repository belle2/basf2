/*
 * BoundedSpaceMap.h
 *
 *  Created on: Jan 26, 2014
 *      Author: kvasnicka
 */

#ifndef BOUNDEDSPACEMAP_H_
#define BOUNDEDSPACEMAP_H_

#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

#include <eutel/merge/CircularTools.h>

/** Container to hold a limited number of the most recently
 * added circular-type values.
 */
struct BoundedSpaceSet {
  typedef std::set<CIRC::tag_type, CIRC::compare_type> set_type;
  typedef std::set<CIRC::tag_type, CIRC::compare_type>::iterator iterator;

  /** Constructor takes the maximum size of the container. */
  BoundedSpaceSet(std::size_t maxSize):
    m_maxSize(maxSize), m_set(&CIRC::compare)
  {}

  /** Set max size of the set.
  * @param maxSize new maximum size to be set.
  */
  inline void setMaxSize(size_t maxSize) { m_maxSize = maxSize; ensureSize(); }

  /** Get number of free positions in the set.
   * @return number of free positions in the map, maxSize - actual size.
   */
  inline std::size_t getFreeSize() const { return m_maxSize - m_set.size(); }

  /** Get the smallest label in the set in the sense of sorting
   * properties of CIRC::tag_type.
   * @return smallest set label
   */
  inline CIRC::tag_type getBottom() const { return *m_set.begin(); }

  /** Get the largest label in the set in the sense of sorting
   * properties of CIRC::tag_type.
   * @return largest set label
   */
  inline CIRC::tag_type getTop() const { return (m_set.empty()) ? 0 : *m_set.rbegin(); }

  /** Get the median (mid-range) value of the set.
   * @return (upper bound on) the median of set elements
   */
  inline CIRC::tag_type getMedian() const
  {
    if (m_set.size() < 5) return 0;
    auto it = m_set.begin(); std::advance(it, m_set.size() / 2);
    return *it;
  }

  /** Check if the buffer is empty.
   * @return true if there are no data in the deque.
   */
  inline bool isEmpty() const { return m_set.size() == 0; }

  /** Insert a new element into the map. If thereby the maximum map size is exceeded,
   * the smallest element is dropped. Caution: the method will not add minimum element
   * to a full buffer.
   * @param k key value of the element
   * @return if there is no element with label k, pair (reference to an empty array at k, true).
   * if there is such an element, the pair (reference to existing array at k, false)
   */
  inline void put(CIRC::tag_type k)
  {
    m_set.insert(k);
    // We don't shrink if adding a minimum element to a full buffer.
    if (getFreeSize() > 0 || k >= getBottom()) ensureSize();
  }

  /** Does the map contain an element with key k?
   * @param k the key to search for
   * @return true if key found, otherwise false.
   */
  inline bool hasKey(CIRC::tag_type k) const { return (m_set.count(k) > 0);  }

  /** Delete element with key k from the set.
   * @param k The key to delete from the set.
   */
  inline void pop(CIRC::tag_type k) { m_set.erase(k); }

private:

  /** Drop extra elements to keep size at or below m_maxSize */
  inline void ensureSize() { while (m_set.size() > m_maxSize) m_set.erase(m_set.begin()); }

  std::size_t
  m_maxSize;  /**< Maximum size of the container; when reached, elements will be dropped to make space for new elements. */
  set_type m_set;         /**< The set holding the values.*/

};


/** Bounded space map: searchable circular buffer.
 * The class holds a map with circular sort order and keeps the specified
 * number of newest elements by
 * 1. replacing smallest elements with incoming elements,
 * 2. discarding retrieved elements.
 */
template < typename timestamp_type, typename item_type >
struct BoundedSpaceMap {

  typedef std::vector<item_type> collection_type;
  typedef std::tuple< timestamp_type, collection_type> map_value_type;
  typedef std::map<CIRC::tag_type, map_value_type, CIRC::compare_type> map_type;
  typedef typename map_type::iterator map_iterator;

  /** Constructor takes max size of the buffer.
   * @param maxSize Maximum size of the buffer (both of the map and deque)
   */
  BoundedSpaceMap(std::size_t maxSize):
    m_maxSize(maxSize), m_map(&CIRC::compare), m_returnVector(collection_type(0)),
    m_lastIter(m_map.end()), m_lastTag(CIRC::c_tagUpperBound)
  {}

  /** Set max size of the map.
   * @param maxSize new maximum size to be set.
   */
  inline void setMaxSize(size_t maxSize) { m_maxSize = maxSize; ensureSize(); }

  /** Get number of free positions in the map.
   * @return number of free positions in the map, maxSize - actual size.
   */
  inline std::size_t getFreeSize() const { return m_maxSize - m_map.size(); }

  /** Get the smallest label in the map in the sense of sorting
   * properties of CIRC::tag_type.
   * @return smallest map+ label
   */

  inline CIRC::tag_type getBottom() const { return m_map.begin()->first; }

  /** Get the largest label in the map in the sense of sorting
   * properties of CIRC::tag_type.
   * @return largest map label
   */
  inline CIRC::tag_type getTop() const { return (m_map.empty() ? 0 : m_map.rbegin()->first); }

  /** Get the median (mid-range) label of the map.
   * @return (upper bound on) the median of map labels
   */
  inline CIRC::tag_type getMedian() const
  {
    // Only report when stable enough
    if (m_map.size() < 5) return 0;
    auto it = m_map.begin(); std::advance(it, m_map.size() / 2);
    return it->first;
  }


  /** Check if the buffer is empty.
   * @return true if there are no data in the deque.
   */
  inline bool isEmpty() const { return m_map.size() == 0; }

  /** Insert a new element into the map. If thereby the maximum map size is exceeded,
   * the smallest element is dropped. Caution: the method will not add minimum element
   * to a full buffer.
   * @param k key value of the element
   * @return if there is no element with label k, pair (reference to an empty array at k, true).
   * if there is such an element, the pair (reference to existing array at k, false)
   */
  inline void put(CIRC::tag_type k, timestamp_type timestamp, collection_type& v)
  {
    // Don't create map[k] if there is nothing to insert
    if (v.size() == 0) return;
    std::get<0>(m_map[k]) = timestamp;
    collection_type& mapv = std::get<1>(m_map[k]);
    mapv.clear();
    for (auto item : v) mapv.push_back(std::move(item));
    // We don't shrink if adding a minimum element to a full buffer.
    if (getFreeSize() > 0 || k >= getBottom()) ensureSize();
  }

  /** Does the map contain an element with key k?
   * @param k the key to search for
   * @return true if key found, otherwise false.
   */
  inline bool hasKey(CIRC::tag_type k)
  {
    auto iv = m_map.find(k);
    if (iv != m_map.end()) {
      m_lastIter = iv;
      m_lastTag = k;
      return true;
    } else
      return false;
  }

  /** Return timestamp of event at k.
   * @param k the key whose timestamp is requested
   * @return the timestamp of event at k, or 0 if not found - check with hasKey!
   */
  timestamp_type getTimeStamp(CIRC::tag_type k)
  {
    map_iterator iv = (m_lastTag == k) ? m_lastIter : m_map.find(k);
    if (iv != m_map.end()) {
      return std::get<0>(iv->second);
      m_lastIter = iv;
      m_lastTag = k;
    } else
      return 0;
  }

  /** Return const reference to the collection at k. If found, the value is
   * removed from the map. If not found, an empty collection is returned.
   * @param k the key whose value is searched for
   * @return const ref to the collection at k, if it exists, otherwise empty collection.
   */
  inline const collection_type& getData(CIRC::tag_type k)
  {
    m_returnVector.clear();
    map_iterator iv = (m_lastTag == k) ? m_lastIter : m_map.find(k);
    if (iv != m_map.end()) {
      for (auto item : std::get<1>(iv->second)) m_returnVector.push_back(std::move(item));
      m_map.erase(iv);
      m_lastIter = m_map.end();
      m_lastTag = CIRC::c_tagUpperBound;
    }
    return m_returnVector;
  }

private:

  inline void ensureSize() { while (m_map.size() > m_maxSize) m_map.erase(m_map.begin()); }

  std::size_t
  m_maxSize;          /**< Maximum allowed size of the map. If reached, oldest events are dropped when new events are added.*/
  map_type m_map;                 /**< The map holding tags, timestamps and collection data.*/
  collection_type m_returnVector; /**< Collection to hold retrieved data, because original entry is deleted on retrieval.*/
  map_iterator m_lastIter;        /**< The last queried position, presumably by hasKey or getTimeStamp, to save map searches. */
  CIRC::tag_type m_lastTag;       /**< Tag of the last queried entry (by hasKey or getTimeStamp), to save map searches. */
};

#endif /* BOUNDEDSPACEMAP_H_ */
