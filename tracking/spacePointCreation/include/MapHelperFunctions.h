/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

# pragma once

#include <string>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <vector>
#include <utility>
#include <tuple>
#include <numeric>

namespace Belle2 {

  /**
   * get the unique keys of a map (i.e. if no multimap is passed all keys are returned)
   * NOTE: for this to work the keys have to have a defined operator < () as std::sort is used!
   * NOTE: as a side effect the keys are returned sorted!
   */
  template <typename MapType>
  std::vector<typename MapType::key_type> getUniqueKeys(const MapType& aMap)
  {
    std::vector<typename MapType::key_type> allKeys; // collect all keys of the map -> then sort & unique (+resize)
    if (aMap.empty()) { return allKeys; }

    typedef typename MapType::const_iterator mapIter;
    for (mapIter it = aMap.begin(); it != aMap.end(); ++it) { allKeys.push_back(it->first); }
    std::sort(allKeys.begin(), allKeys.end());
    auto newEnd = std::unique(allKeys.begin(), allKeys.end());
    allKeys.resize(std::distance(allKeys.begin(), newEnd));

    return allKeys;
  }

  /**
   * get the number of unique keys inside the map
   * NOTE: for non-multimap this is the same as .size()
   */
  template <typename MapType>
  unsigned int getUniqueSize(const MapType& aMap) { return getUniqueKeys<MapType>(aMap).size(); }

  /**
   * get the unique keys of a map together with the number of values associated to each key. first elements are keys, second are number of values
   * NOTE: for a non-multimap this returns all keys with a .second of 1!
   */
  template <typename MapType>
  std::vector<std::pair<typename MapType::key_type, unsigned int> > getNValuesPerKey(const MapType& aMap)
  {
    typedef typename MapType::key_type keyT;
    typedef typename MapType::const_iterator mapIter;

    std::vector<std::pair<keyT, unsigned int> > valuesPerKey;
    if (aMap.empty()) return valuesPerKey; // return empty vector if map is empty

    std::vector<keyT> uniqueKeys = getUniqueKeys<MapType>(aMap);

    for (keyT key : uniqueKeys) {
      std::pair<mapIter, mapIter> keyRange = aMap.equal_range(key);
      valuesPerKey.push_back(std::make_pair(key, std::distance(keyRange.first, keyRange.second)));
    }
    return valuesPerKey;
  }

  /**
   * get all values stored in the map for a given key
   */
  template <typename MapType>
  std::vector<typename MapType::mapped_type> getValuesToKey(const MapType& aMap, typename MapType::key_type aKey)
  {
    typedef typename MapType::const_iterator mapIter;

    std::vector<typename MapType::mapped_type> values;
    if (aMap.empty()) return values;

    std::pair<mapIter, mapIter> keyRange = aMap.equal_range(aKey);
    for (mapIter it = keyRange.first; it != keyRange.second; ++it) { values.push_back(it->second); }

    return values;
  }

  /**
   * get the (key, value, number of values) tuples stored in the map, sorted after the following scheme (descending order)
   * 1) the number of associated values to one key
   * 2) the sum of the associated values to that key
   * NOTE: for a non-multimap this returns the content of the map ordered by valued
   * CAUTION: if one of the values to a key is NaN this key will be the first (of the ones with the same number of associated values)
   * @returns a vector of tuples where get<0> is the key, get<1> is the sum of the values to the key and get<2> is the number of values to the key
   */
  template <typename MapType>
  std::vector<std::tuple<typename MapType::key_type, typename MapType::mapped_type, unsigned int> >
  getSortedKeyValueTuples(const MapType& aMap)
  {
    typedef typename MapType::key_type keyT;
    typedef typename MapType::mapped_type mapT;

    std::vector<std::tuple<keyT, mapT, unsigned int> > keyValuePairs;
    if (aMap.empty()) return keyValuePairs; // return empty vector if nothing is stored in map

    std::vector<std::pair<keyT, unsigned int> > nValuesPerKey = getNValuesPerKey(aMap);

    for (std::pair<keyT, unsigned int> keyValues : nValuesPerKey) {
      std::vector<mapT> valuesToKey = getValuesToKey(aMap, keyValues.first);

      mapT valueSum = std::accumulate(valuesToKey.begin(), valuesToKey.end(), 0.0);
      keyValuePairs.push_back(std::make_tuple(keyValues.first, valueSum, keyValues.second));
    }

    // sort using a lambda function (using std::tie as std::tuple has a defined operator < that ensures strict weak ordering)
    std::sort(keyValuePairs.begin(), keyValuePairs.end(),
              [](const std::tuple<keyT, mapT, unsigned int>& lTuple, const std::tuple<keyT, mapT, unsigned int>& rTuple)
    { return std::tie(std::get<2>(rTuple), std::get<1>(rTuple)) < std::tie(std::get<2>(lTuple), std::get<1>(lTuple)); }
             );

    return keyValuePairs;
  }

  /**
   * get all values in the map (i.e. dump out all values that are stored in the map). The connection to the keys is lost!
   * TODO: write test for this!!!!
   */
  template<typename MapType>
  std::vector<typename MapType::mapped_type> getAllValues(const MapType& aMap)
  {
    typedef typename MapType::key_type keyT;
    std::vector<keyT> allKeys = getUniqueKeys(aMap);

    typedef typename MapType::mapped_type valueT;
    std::vector<valueT> allValues;
    for (const keyT& key : allKeys) {
      std::vector<valueT> keyValues = getValuesToKey(aMap, key);
      for (const valueT& value : keyValues) {
        allValues.push_back(value);
      }
    }

    return allValues;
  }

  /**
   * get the contents of the map as string. NOTE: should compile without warning for any map (e.g. map, multimap, unordered_map,...) with key and values of a type that have a defined stream insertion operator (only tested for multimap and unordered_multimap!)
   */
  template <typename MapType>
  std::string printMap(const MapType& aMap)
  {
    if (aMap.empty()) return std::string("passed map is empty!");

    typedef typename MapType::key_type keyT;
    typedef typename MapType::mapped_type mapT;

    std::stringstream mapContent;
    mapContent << "content of map:\n";
    for (keyT key : getUniqueKeys(aMap)) {
      mapContent << "key: " << key << " -> value(s):";
      for (mapT value : getValuesToKey(aMap, key)) { mapContent << " " << value; }
      mapContent << "\n";
    }

    return mapContent.str() + "\n"; // terminate with endline
  }

}
