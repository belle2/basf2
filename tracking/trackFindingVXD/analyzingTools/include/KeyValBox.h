/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// stl:
#include <vector>
#include <utility> // for pair
#include <algorithm> // for find_if


namespace Belle2 {

  /** Minimal container storing a pair of < KeyType, ValueType> */
  template<class KeyType, class ValueType>
  class KeyValBox {
  protected:

    /** the container containing the keys and values */
    std::vector<std::pair<KeyType, ValueType> > m_container;

  public:

    /** typedef for readable entry-type */
    using BoxEntry = std::pair<KeyType, ValueType>;


    /** typedef for more readable iterator-type */
    using Iterator = typename std::vector<BoxEntry>::iterator;


    /** typedef for more readable iterator-type */
    using ConstIterator = typename std::vector<BoxEntry>::const_iterator;


    /** for given key a pointer to the value is returned. If key was invalid, a nullptr-ptr will be returned */
    ValueType* find(const KeyType& aKey)
    {
      Iterator foundPos = std::find_if(
                            m_container.begin(),
                            m_container.end(),
                            [&](const BoxEntry & entry) -> bool
      { return entry.first == aKey; }
                          );
      return (foundPos == m_container.end() ? nullptr : &foundPos->second);
    }


    /** push_back for new pair given */
    void push_back(std::pair<KeyType, ValueType>& newPair) { m_container.push_back(newPair); }


    /** push_back for new pair given */
    void push_back(std::pair<KeyType, ValueType> newPair) { m_container.push_back(newPair); }


    /** returns iterator for container: begin */
    Iterator begin() { return m_container.begin(); }


    /** returns iterator for container: begin */
    ConstIterator begin() const { return m_container.begin(); }


    /** returns iterator for container: end */
    Iterator end() { return m_container.end(); }


    /** returns iterator for container: end */
    ConstIterator end() const { return m_container.end(); }


    /** returns number of entries in container: */
    unsigned int size() const { return m_container.size(); }
  };

}
