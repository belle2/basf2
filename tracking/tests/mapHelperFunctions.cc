/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// google test framework
#include <gtest/gtest.h>

#include <tracking/spacePointCreation/MapHelperFunctions.h>
#include <framework/logging/Logger.h> // for B2INFO, etc...

#include <unordered_map>
// #include <map> // only testing unordered maps here, but should be the same for maps (only slower)
#include <complex>
#include <cmath>
#include <boost/concept_check.hpp>

using namespace std;

namespace Belle2 {

  typedef std::unordered_multimap<int, double> i2dMultiMap; /**< typedef for less writing effort */
  typedef std::unordered_map<int, double> i2dMap; /**< typedef for less writing effort */

  /** create a multimap with @param nEntries entries, that show possibly problematic behavior (concerning strict weak ordering)
   * the i-th entry is: [(i % 6), std::acos( (i % 5) * 0.3 )
   * NOTE: that the value can be NaN for some cases!
   */
  template<typename Functor>
  i2dMultiMap createMultiMap(unsigned int nEntries, Functor funct)
  {
    i2dMultiMap map;
    for (unsigned int i = 0; i < nEntries; ++i) {
      map.insert(std::make_pair((i % 6), funct.operator()(i)));
    }
    return map;
  }

  /** create a multimap with
   * @param nEntries entries,
   * @param funct Function Object that determines how the value to a given key is calculated (
   * for this method to work: operator must take an unsigned int as argument and return a double from that)
   * NOTE: that the value can be NaN for some cases!
   */
  template<typename Functor>
  i2dMap createMap(unsigned int nEntries, Functor funct)
  {
    i2dMap map;
    for (unsigned int i = 0; i < nEntries; ++i) {
      map.insert(std::make_pair(i, funct.operator()(i)));
    }
    return map;
  }

  /** function object that implements: acos( ( i % 5) * .3 ), which produces some NaNs but no negative numbers */
  struct {
    double operator()(unsigned int i)
    {
      return std::acos((i % 5) * .3);
    }
  } acosMod5;

  /** function object that implementes sin(i/2). This does give positive and negative weights, but no NaNs */
  struct {
    double operator()(unsigned int i)
    {
      return std::sin(i * .5);
    }
  } sinHalf;

  /**
   * class for testing the helper functions from MapHelperFunctions.h
   */
  class MapHelperFunctionsTest : public ::testing::Test {
  protected:
    virtual void SetUp()
    {
      _nEntries = 20;
      _map = createMap(_nEntries, sinHalf);
      _multimap = createMultiMap(_nEntries, sinHalf);
      _nanMap = createMap(_nEntries, acosMod5);
      _nanMultiMap = createMultiMap(_nEntries, acosMod5);
    }

    i2dMap _map;
    i2dMultiMap _multimap;
    i2dMultiMap _nanMultiMap;
    i2dMap _nanMap;
    unsigned int _nEntries;
  };

  /** test the methods that are use to create the maps for the later tests */
  TEST_F(MapHelperFunctionsTest, testCreatorFunctions)
  {
    unsigned int N = _nEntries; // same number as in setup
    EXPECT_EQ(N, _nanMultiMap.size()); // test if creation worked (very basically by comparing the size of the returned _nanMap)
    EXPECT_EQ(N, _nanMap.size());
    EXPECT_EQ(N, _multimap.size());
    EXPECT_EQ(N, _map.size());

//     std::cout << printMap(_nanMap) << std::endl;
//     std::cout << printMap(_nanMultiMap) << std::endl;
//     std::cout << printMap(_multimap) << std::endl;
//     std::cout << printMap(_map) << std::endl;

    // do some hard-coded queries to see if the _nanMaps were filled according to specified method
    std::vector<int> possibleKeys = { 0, 1, 2, 3, 4, 5 };
    for (int key : possibleKeys) {
      std::cout << " key " << key << std::endl;
      auto range = _nanMultiMap.equal_range(key);
      i2dMultiMap::iterator multiMapIt = range.first;
      for (int i = std::distance(range.first, range.second) - 1; i != 0; --i) {
        if (std::isnan(multiMapIt->second)) {
          B2INFO("Not Comparing value to key " << i << " because value is NaN! (on purpose!)")
          continue;
        }
        EXPECT_DOUBLE_EQ(multiMapIt->second, acosMod5.operator()(key + i));
        ++multiMapIt;
      }

      // TODO: test second multimap creation
//       range = _multimap.equal_range(key);
//       multiMapIt = range.first;
//       for(int i = std::distance(range.first, range.second) - 1; i != 0; --i) {
//  EXPECT_DOUBLE_EQ(multiMapIt->second, sinHalf.operator()(key*5 + i));
//  ++multiMapIt;
//       }
    }

    i2dMap::iterator _nanMapIt;
    for (unsigned int i = 0; i < N; ++i) {
      auto range = _nanMap.equal_range(i);
      i2dMap::iterator mapIt = range.first;
      for (unsigned int j = 0; j < std::distance(range.first, range.second); ++j) {
        if (std::isnan(mapIt->second)) {
          B2INFO("Not Comparing value to key " << i << " because acos( " << i << " % 5) *. 3) is NaN! (on purpose!)");
          continue; // do not compare NaNs! (will always fail due to their nature)
        }
        EXPECT_DOUBLE_EQ(mapIt->second, acosMod5.operator()(i));
        ++mapIt;
      }

      range = _map.equal_range(i);
      mapIt = range.first;
      for (unsigned int j = 0; j < std::distance(range.first, range.second); ++j) {
        EXPECT_DOUBLE_EQ(mapIt->second, sinHalf.operator()(i));
      }
    }
  }

  /** test if the 'getUniqueKeys' method returns the right values */
  TEST_F(MapHelperFunctionsTest, testGetUniqueKeys)
  {
    // get the unique keys for the map (assuming that the key_type is int!)
    std::vector<int> uniqueKeys = getUniqueKeys(_map);
    EXPECT_EQ(uniqueKeys.size(), _map.size()); // for the map the number of uniqueKeys has to be the same as the size!
    for (size_t i = 0; i < uniqueKeys.size(); ++i) {
      // using std::find because in this way there is no assumption on the order of the keys!
      auto findIt = std::find(uniqueKeys.begin(), uniqueKeys.end(), i);
      EXPECT_FALSE(findIt == uniqueKeys.end());
    }

    // get the unique keys for the multimap
    std::vector<int> uniqueMultiKeys = getUniqueKeys(_multimap);
    unsigned int expectedSize = _multimap.size() > 6 ? 6 : _multimap.size(); // (creation of the map is % 6 -> only 6 keys possible)
    EXPECT_EQ(expectedSize, uniqueMultiKeys.size());
    for (size_t i = 0; i < uniqueMultiKeys.size(); ++i) {
      // using std::find because in this way there is no assumption on the order of the keys!
      auto findIt = std::find(uniqueMultiKeys.begin(), uniqueMultiKeys.end(), i);
      EXPECT_FALSE(findIt == uniqueMultiKeys.end());
    }
  }

  /** test the 'getNValuesPerKey' method */
  TEST_F(MapHelperFunctionsTest, testGetNValuesPerKey)
  {
    // TODO
  }

  /** test if the 'getSortedKeyValuePairs' method actually works as advertised */
  TEST_F(MapHelperFunctionsTest, testGetSortedKeyValuePairs)
  {
    // TODO
  }
}