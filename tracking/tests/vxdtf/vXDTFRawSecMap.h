#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/VXDTFRawSecMap.h>
#include <limits>       // std::numeric_limits
#include <gtest/gtest.h>
#include <boost/concept_check.hpp>

#include <TRandom3.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class VXDTFRawSecMapTest : public ::testing::Test {

  public:

    typedef std::pair<double, double> PairDouble; /**< shortcut for a pair of double */

// member-functions are defined within this file after the tests
    /** initilizes most needed values for the tests */
    void initializeRawMap(PairDouble quantiles = std::make_pair(0, 1),
                          PairDouble smallQuantiles = std::make_pair(0, 1),
                          double minSampleThreshold = 1,
                          double smallSampleThreshold = 10,
                          double smallStretchFactor = 0,
                          double stretchFator = 0);


    /** creates a gaussian distributed sample of values and returns their biggest and smallest value */
    void createSample(double sampleSize,
                      double meanValue,
                      double sigmaValue,
                      double& smallestValue,
                      double& biggestValue,
                      VXDTFRawSecMap::CutoffValues& sample);


    /** expects a sorted sample and a pair of quantiles (.first is the smaller one) between 0 and 1 */
    PairDouble findQuantilesInSample(VXDTFRawSecMap::CutoffValues& sample,
                                     PairDouble quantiles = std::make_pair(0, 1));


    /** the map which shall be tested */
    VXDTFRawSecMap m_RawMap;
  protected:
  };





  /** Test cutoff extraction for easy cutoff-quantiles. */
  TEST_F(VXDTFRawSecMapTest, testCalcCutoffsSimpleCase)
  {
    /** contains all cutoff-Values collected during run, is a list since it has to be sorted in the end: */
    VXDTFRawSecMap::CutoffValues aSample, aSmallSample;
    int smallSampleSize = 8, sampleSize = 100;
    double meanValue = 1,
           sigmaValue = 0.4, // small but possible probability of negative numbers for big sampleSize
           smallestValue = std::numeric_limits<int>::max(), // no typo, will be replaces by smallest value of sample
           biggestValue = std::numeric_limits<int>::min();  // no typo, will be replaces by biggest value of sample

    initializeRawMap(); // fill in standard-values

    // fill big sample with randomly generated values picked from a normal distribution:
    createSample(sampleSize, meanValue, sigmaValue, smallestValue, biggestValue, aSample);

    PairDouble myResult = m_RawMap.calcCutoffs(aSample);

    EXPECT_EQ(smallestValue, myResult.first);
    EXPECT_EQ(biggestValue, myResult.second);

    // extra check for test-member:
    PairDouble foundQuantiles = findQuantilesInSample(aSample, std::make_pair(0, 1));
    EXPECT_EQ(smallestValue, foundQuantiles.first);
    EXPECT_EQ(biggestValue, foundQuantiles.second);


    // fill small sample with randomly generated values picked from a normal distribution:
    createSample(smallSampleSize, meanValue, sigmaValue, smallestValue, biggestValue, aSmallSample);

    PairDouble mySmallResult = m_RawMap.calcCutoffs(aSmallSample);

    EXPECT_EQ(smallestValue, mySmallResult.first);
    EXPECT_EQ(biggestValue, mySmallResult.second);

    // extra check for test-member:
    PairDouble foundQuantiles2 = findQuantilesInSample(aSmallSample, std::make_pair(0, 1));
    EXPECT_EQ(smallestValue, foundQuantiles2.first);
    EXPECT_EQ(biggestValue, foundQuantiles2.second);
  }



  /** Test cutoff extraction for random cutoff-quantiles. */
  TEST_F(VXDTFRawSecMapTest, testCalcCutoffsRandomCase)
  {
    /** contains all cutoff-Values collected during run, is a list since it has to be sorted in the end: */
    VXDTFRawSecMap::CutoffValues aSample, aSmallSample, aBigSample;
    int smallSampleSize = 22, sampleSize = 100, bigSampleSize = 5555;
    double meanValue = 1,
           sigmaValue = 0.4, // small but possible probability of negative numbers for big sampleSize
           smallestValue = std::numeric_limits<int>::max(), // no typo, will be replaces by smallest value of sample
           biggestValue = std::numeric_limits<int>::min();  // no typo, will be replaces by biggest value of sample
    PairDouble quantiles = std::make_pair(0.02, 0.91),
               smallQuantiles = std::make_pair(0.02, 0.91);
    initializeRawMap(
      quantiles, // quantiles
      smallQuantiles, // smallQuantiles
      1, // minSampleThreshold
      50 // smallSampleThreshold, rest stays at standard-values
    ); // fill special values


    // find index-positions for given quantiles:

    // fill medium sample with randomly generated values picked from a normal distribution:
    createSample(sampleSize, meanValue, sigmaValue, smallestValue, biggestValue, aSample);

    PairDouble myResult = m_RawMap.calcCutoffs(aSample);

    PairDouble expectedValues = findQuantilesInSample(aSample, quantiles);
    EXPECT_EQ(expectedValues.first, myResult.first);
    EXPECT_EQ(expectedValues.second, myResult.second);


    // fill small sample with randomly generated values picked from a normal distribution:
    createSample(smallSampleSize, meanValue, sigmaValue, smallestValue, biggestValue, aSmallSample);

    PairDouble mySmallResult = m_RawMap.calcCutoffs(aSmallSample);

    PairDouble smallExpectedValues = findQuantilesInSample(aSmallSample, smallQuantiles);
    EXPECT_EQ(smallExpectedValues.first, mySmallResult.first);
    EXPECT_EQ(smallExpectedValues.second, mySmallResult.second);


    // fill big sample with randomly generated values picked from a normal distribution:
    createSample(bigSampleSize, meanValue, sigmaValue, smallestValue, biggestValue, aBigSample);

    PairDouble myBigResult = m_RawMap.calcCutoffs(aBigSample);

    PairDouble bigExpectedValues = findQuantilesInSample(aBigSample, quantiles);
    EXPECT_EQ(bigExpectedValues.first, myBigResult.first);
    EXPECT_EQ(bigExpectedValues.second, myBigResult.second);
  }




  /*************** Member-function-definitions ******************/


  void VXDTFRawSecMapTest::initializeRawMap(PairDouble quantiles,
                                            PairDouble smallQuantiles,
                                            double minSampleThreshold,
                                            double smallSampleThreshold,
                                            double smallStretchFactor,
                                            double stretchFator)
  {
    m_RawMap.setCutoffQuantiles(quantiles);
    m_RawMap.setSmallCutoffQuantiles(smallQuantiles);
    m_RawMap.setSampleThreshold(smallSampleThreshold);
    m_RawMap.setMinSampleThreshold(minSampleThreshold);
    m_RawMap.setStretchFactor(stretchFator);
    m_RawMap.setSmallStretchFactor(smallStretchFactor);
  }



  void VXDTFRawSecMapTest::createSample(double sampleSize,
                                        double meanValue,
                                        double sigmaValue,
                                        double& smallestValue,
                                        double& biggestValue,
                                        VXDTFRawSecMap::CutoffValues& sample)
  {
    TRandom3 aRng;

    smallestValue = std::numeric_limits<int>::max();
    biggestValue = std::numeric_limits<int>::min();

    double sampleVal = 0;
    for (int i = 0; i < sampleSize; ++i) {
      sampleVal = aRng.Gaus(meanValue, sigmaValue);
      if (sampleVal < smallestValue) { smallestValue = sampleVal; }
      if (sampleVal > biggestValue) { biggestValue = sampleVal; }
      sample.push_back(sampleVal);
    }
    sample.sort();
  }



  VXDTFRawSecMapTest::PairDouble VXDTFRawSecMapTest::findQuantilesInSample(VXDTFRawSecMap::CutoffValues& sample,
      PairDouble quantiles)
  {
    int sampleSize = sample.size();
    double dSampleSize = double(sampleSize);

    int lowerIndex = static_cast<int>(dSampleSize * quantiles.first + 0.5);
    int higherIndex = static_cast<int>(dSampleSize * quantiles.second - 0.5); // maxIndex is sampleSize - 1, therefore -0.5

    if (lowerIndex < 0 or lowerIndex > sampleSize) { B2FATAL("lowerIndes is crap, (bounds are 0 and " << sampleSize << ", is: " << lowerIndex << ")");}
    if (higherIndex < 0 or higherIndex > sampleSize) { B2FATAL("higherIndex is crap, (bounds are 0 and " << sampleSize << ", is: " << higherIndex << ")");}

    VXDTFRawSecMap::CutoffValues::iterator lowerIter = sample.begin();
    for (int i = 0 ; i < lowerIndex; i++) { ++lowerIter; }

    VXDTFRawSecMap::CutoffValues::reverse_iterator higherIter = sample.rbegin();
    for (int i = 0 ; i < (sampleSize - 1 - higherIndex); i++) { ++higherIter; }

    B2INFO("findQuantilesInSample: sampleSize: " << sampleSize << ", quantiles: " << quantiles.first << "/" << quantiles.second <<
           ", found indices: " << lowerIndex << "/" << higherIndex << ", found values: " << *lowerIter << "/" << *higherIter);
    return std::make_pair(*lowerIter, *higherIter);
  }
}
