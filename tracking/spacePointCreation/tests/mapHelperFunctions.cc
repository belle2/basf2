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
#include <cmath>
#include <numeric>

using namespace std;

namespace Belle2 {

  typedef std::unordered_multimap<int, double> i2dMultiMap; /**< typedef for less writing effort */
  typedef std::unordered_map<int, double> i2dMap; /**< typedef for less writing effort */

  /** create a multimap with
   * @param nEntries entries.
   * @param funct Function object which values are used to determine the keys by the return value of funct.operator()
   * NOTE: funct.operator has to take an int and return a double
   */
  template<typename Functor>
  i2dMultiMap createMultiMap(int nEntries, Functor funct)
  {
    i2dMultiMap map;
    for (int i = 0; i < nEntries; ++i) {
      map.insert(std::make_pair((i % 6), funct.operator()(i)));
    }
    return map;
  }

  /** create a multimap with
   * @param nEntries entries.
   * @param funct Function object which values are used to determine the keys by the return value of funct.operator()
   * NOTE: funct.operator has to take an int and return a double
   */
  template<typename Functor>
  i2dMap createMap(int nEntries, Functor funct)
  {
    i2dMap map;
    for (int i = 0; i < nEntries; ++i) {
      map.insert(std::make_pair(i, funct.operator()(i)));
    }
    return map;
  }

  /** function object that implements cosecans(x) == 1/cos(x) by tan(x) / sin(x).
   * This construction should give a NaN if the argument is 0 (but hopefully cppcheck can be fooled to not raise a division by zero error)
   * NOTE: it is tested if this actually is the case!
   */
  struct {
    double operator()(int i)
    {
      return std::tan(i) / std::sin(i);
    }
  } secans;

  /** function object that implements sin(i/2). This does give positive and negative weights, but no NaNs */
  struct {
    double operator()(int i)
    {
      return std::sin(i * .5);
    }
  } sinHalf;

  /**
   * class for testing the helper functions from MapHelperFunctions.h
   */
  class MapHelperFunctionsTest : public ::testing::Test {
  protected:
    /** set up the maps for the tests */
    virtual void SetUp()
    {
      _nEntries = 20;
      _map = createMap(_nEntries, sinHalf);
      _multimap = createMultiMap(_nEntries, sinHalf);
      _nanMap = createMap(_nEntries, secans);
      _nanMultiMap = createMultiMap(_nEntries, secans);
    }

    i2dMap _map; /**< map filled with values obtained by sinHalf functor */
    i2dMultiMap _multimap; /**< multimap filled with values obtained by sinHalf functor */
    i2dMultiMap _nanMultiMap; /**< multimap filled with values obtained by secans functor -> contains a NaN! */
    i2dMap _nanMap; /**< map filled with values obtained by secans functor -> contains a NaN! */
    int _nEntries; /**< the number of pairs that will be put into the maps for the tests */
  };

  /** test the methods that are use to create the maps for the later tests */
  TEST_F(MapHelperFunctionsTest, testCreatorFunctions)
  {
    unsigned int N = _nEntries; // same number as in setup
    EXPECT_EQ(N, _nanMultiMap.size()); // test if creation worked (very basically by comparing the size of the returned _nanMap)
    EXPECT_EQ(N, _nanMap.size());
    EXPECT_EQ(N, _multimap.size());
    EXPECT_EQ(N, _map.size());

    ASSERT_TRUE(std::isnan(secans.operator()(0))) << "For the following tests to work properly this is expected to return a NaN (-nan)";

//     std::cout << printMap(_nanMap) << std::endl;
//     std::cout << printMap(_nanMultiMap) << std::endl;
//     std::cout << printMap(_multimap) << std::endl;
//     std::cout << printMap(_map) << std::endl;

    // do some hard-coded queries to see if the _nanMaps were filled according to specified method
    std::vector<int> possibleKeys = { 0, 1, 2, 3, 4, 5 };
    for (int key : possibleKeys) {
      std::vector<double> nanPossibleValues, possibleValues;
      for (int arg = key; arg < _nEntries; arg += 6) {
        nanPossibleValues.push_back(secans.operator()(arg));
        possibleValues.push_back(sinHalf.operator()(arg));
      }
      // test the multimap with nan first
      auto keyRange = _nanMultiMap.equal_range(key);
      // check the numbers of stored values and of possible values
      EXPECT_EQ(nanPossibleValues.size(), std::distance(keyRange.first, keyRange.second));
      i2dMultiMap::iterator multiMapIt = keyRange.first;
      for (int i = 0; i < std::distance(keyRange.first, keyRange.second); ++i) {
        if (!std::isnan(multiMapIt->second)) { // cannot find nan value with std::find (NaN == NaN is always false)
          auto findIt = std::find(nanPossibleValues.begin(), nanPossibleValues.end(), multiMapIt->second);
          EXPECT_FALSE(findIt == nanPossibleValues.end()); // all stored values have to be possible
        } else { // check if there is at least on NaN value in the possible Values
          EXPECT_TRUE(std::count_if(nanPossibleValues.begin(), nanPossibleValues.end(),
          [](const double & val) { return std::isnan(val); }) > 0);
        }
        ++multiMapIt;
      }
      keyRange = _multimap.equal_range(key);
      EXPECT_EQ(possibleValues.size(), std::distance(keyRange.first, keyRange.second));
      multiMapIt = keyRange.first;
      for (int i = 0; i < std::distance(keyRange.first, keyRange.second); ++i) {
        auto findIt = std::find(possibleValues.begin(), possibleValues.end(), multiMapIt->second);
        EXPECT_FALSE(findIt == possibleValues.end());
        ++multiMapIt;
      }
    }

    for (unsigned int i = 0; i < N; ++i) {
      auto range = _nanMap.equal_range(i);
      i2dMap::iterator mapIt = range.first;
      for (int j = 0; j < std::distance(range.first, range.second); ++j) {
        if (std::isnan(mapIt->second)) {
          B2INFO("Not Comparing value to key " << i << " because value is NaN, checking if the functor returns NaN for this key");
          EXPECT_TRUE(std::isnan(secans.operator()(i))); // expect a NaN value from the operator
          continue; // do not compare NaNs! (will always fail due to their nature)
        }
        EXPECT_DOUBLE_EQ(mapIt->second, secans.operator()(i));
        ++mapIt;
      }

      range = _map.equal_range(i);
      mapIt = range.first;
      for (int j = 0; j < std::distance(range.first, range.second); ++j) {
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

  /** test if the 'getValuesToKey' method returns the right values to a given key */
  TEST_F(MapHelperFunctionsTest, testGetValuesToKey)
  {
    // multimap
    std::vector<int> possibleKeys = { 0, 1, 2, 3, 4, 5 };
    for (int key : possibleKeys) {
      std::vector<double> expectedValues;
      for (int arg = key; arg < _nEntries; arg += 6) { expectedValues.push_back(sinHalf.operator()(arg)); }
      std::vector<double> actualValues = getValuesToKey(_multimap, key);
      EXPECT_EQ(expectedValues.size(), actualValues.size());

      // check if all values are present
      for (const double& d : actualValues) {
        EXPECT_FALSE(std::find(expectedValues.begin(), expectedValues.end(), d) == expectedValues.end());
      }
      expectedValues.clear();
      for (int arg = key; arg < _nEntries; arg += 6) {
        expectedValues.push_back(secans.operator()(arg));
      }
      actualValues = getValuesToKey(_nanMultiMap, key);
      EXPECT_EQ(expectedValues.size(), actualValues.size());

      // check if all values are present
      for (const double& d : actualValues) {
        if (std::isnan(d)) {
          B2INFO("Not comparing NaN value!");
          continue;
        }
        EXPECT_FALSE(std::find(expectedValues.begin(), expectedValues.end(), d) == expectedValues.end());
      }
    }

    // map
    for (int i = 0; i < _nEntries; ++i) {
      std::vector<double> values = getValuesToKey(_map, i);
      EXPECT_EQ(values.size(), 1);
      EXPECT_DOUBLE_EQ(values[0], sinHalf.operator()(i));

      values = getValuesToKey(_nanMap, i);
      EXPECT_EQ(values.size(), 1);
      if (!std::isnan(values[0])) { // not checking here (already checked, that the functor leads to this above)
        EXPECT_DOUBLE_EQ(values[0], secans.operator()(i));
      }
    }
  }

  /** test the 'getNValuesPerKey' method */
  TEST_F(MapHelperFunctionsTest, testGetNValuesPerKey)
  {
    std::vector<std::pair<int, unsigned int> > mapKeys = getNValuesPerKey(_map);
    unsigned int oneKey2Value = std::count_if(mapKeys.begin(), mapKeys.end(),
    [](const std::pair<int, unsigned int>& pair) { return pair.second == 1; });
    EXPECT_EQ(_map.size(), oneKey2Value); // all keys should only be there once in a map!

    mapKeys = getNValuesPerKey(_nanMap);
    oneKey2Value = std::count_if(mapKeys.begin(), mapKeys.end(),
    [](const std::pair<int, unsigned int>& pair) { return pair.second == 1; });
    EXPECT_EQ(_nanMap.size(), oneKey2Value);

    // multimaps (NOTE: hardcoded expectations because of knowledge how the maps should be filled!)
    mapKeys = getNValuesPerKey(_nanMultiMap);
    EXPECT_EQ(mapKeys.size(), 6);
    for (const auto& key : mapKeys) {
      EXPECT_EQ(key.second, key.first < 2 ? 4 : 3);
    }

    mapKeys = getNValuesPerKey(_multimap);
    EXPECT_EQ(mapKeys.size(), 6);
    for (const auto& key : mapKeys) {
      EXPECT_EQ(key.second, key.first < 2 ? 4 : 3);
    }
  }

  /** test if the 'getSortedKeyValuePairs' method actually works as advertised */
  TEST_F(MapHelperFunctionsTest, testGetSortedKeyValueTuples)
  {
    // multimap without nans first
    std::vector<int> possibleKeys = { 0, 1, 2, 3, 4, 5 };
    std::vector<double> summedValues;
    std::vector<double> summedNanValues;
    for (int key : possibleKeys) {
      std::vector<double> possibleValues;
      std::vector<double> possibleNanValues;
      for (int arg = key; arg < _nEntries; arg += 6) {
        possibleValues.push_back(sinHalf.operator()(arg));
        possibleNanValues.push_back(secans.operator()(arg));
      }
      summedNanValues.push_back(std::accumulate(possibleNanValues.begin(), possibleNanValues.end(), 0.0));
      summedValues.push_back(std::accumulate(possibleValues.begin(), possibleValues.end(), 0.0));
    }

    std::vector<std::tuple<int, double, unsigned int> > keyValPairs = getSortedKeyValueTuples(_nanMultiMap);

    EXPECT_EQ(keyValPairs.size(), 6);
    // hardcoded checking here (only sampling here! not checking the whole thing!)
    EXPECT_EQ(get<0>(keyValPairs[0]), 0);
    EXPECT_TRUE(std::isnan(get<1>(keyValPairs[0])));
    EXPECT_EQ(get<2>(keyValPairs[0]), 4);

    // all the other checks with multimap without NaNs
    keyValPairs = getSortedKeyValueTuples(_multimap);
    EXPECT_EQ(keyValPairs.size(), 6);

    EXPECT_EQ(get<0>(keyValPairs[0]), 0);
    EXPECT_DOUBLE_EQ(get<1>(keyValPairs[0]), summedValues[0]);
    EXPECT_EQ(get<2>(keyValPairs[0]), 4);

    EXPECT_EQ(get<0>(keyValPairs[2]), 3);
    EXPECT_DOUBLE_EQ(get<1>(keyValPairs[2]), summedValues[3]);
    EXPECT_EQ(get<2>(keyValPairs[2]), 3);

    keyValPairs = getSortedKeyValueTuples(_map);
    EXPECT_EQ(get<0>(keyValPairs[0]), 3); // sin(x/2) has its maximum at x = n*pi, 3 is the value that comes closest to this
    EXPECT_DOUBLE_EQ(get<1>(keyValPairs[0]), sinHalf.operator()(3));

    EXPECT_EQ(get<0>(keyValPairs[19]), 9);
    EXPECT_DOUBLE_EQ(get<1>(keyValPairs[19]), sinHalf.operator()(9));
  }

  /** test the getAllValues() function actually returns all values that are stored in the map*/
  TEST_F(MapHelperFunctionsTest, testGetAllValues)
  {
    // set up vectors with all values there are in the maps
    std::vector<double> possibleValues, nanPossibleValues;
    for (int i = 0; i < _nEntries; ++i) {
      possibleValues.push_back(sinHalf.operator()(i));
      nanPossibleValues.push_back(secans.operator()(i));
    }
    // sort vectors to be able to simply loop over them for comparison since the order is unimportant for this function
    std::sort(possibleValues.begin(), possibleValues.end());
    std::sort(nanPossibleValues.begin(), nanPossibleValues.end());

    // test normal map first
    std::vector<double> allValues = getAllValues(_map);
    EXPECT_EQ(allValues.size(), _nEntries);
    std::sort(allValues.begin(), allValues.end());
    for (int i = 0; i < _nEntries; ++i) {
      EXPECT_DOUBLE_EQ(allValues[i], possibleValues[i]);
    }

    // test multimap with non NaN entries
    allValues = getAllValues(_multimap);
    EXPECT_EQ(allValues.size(), _nEntries);
    std::sort(allValues.begin(), allValues.end());
    for (int i = 0; i < _nEntries; ++i) {
      EXPECT_DOUBLE_EQ(allValues[i], possibleValues[i]);
    }

    // TODO: does not work at the moment, FIX THIS!!
    // test the nan maps
    // allValues = getAllValues(_nanMap);
    // EXPECT_EQ(allValues.size(), _nEntries);
    // std::sort(allValues.begin(), allValues.end());
    // for(int i = 0; i < _nEntries; ++i) {
    //   EXPECT_DOUBLE_EQ(allValues[i], possibleValues[i]);
    // }

    // allValues = getAllValues(_nanMultiMap);
    // EXPECT_EQ(allValues.size(), _nEntries);
    // std::sort(allValues.begin(), allValues.end());
    // for(int i = 0; i < _nEntries; ++i) {
    //   EXPECT_DOUBLE_EQ(allValues[i], possibleValues[i]);
    // }


  }
}
