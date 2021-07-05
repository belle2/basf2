/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/sectorMapTools/MinMaxCollector.h>

#include <TRandom.h>
// #include <tracking/spacePointCreation/SpacePoint.h>
// #include <tracking/trackFindingVXD/filterMap/fourHitVariables/DeltaPt.h>
// #include <tracking/trackFindingVXD/filterMap/fourHitVariables/DeltaDistCircleCenter.h>
// #include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
//
// #include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>
//
// #include <vxd/geometry/SensorInfoBase.h>
//
// #include <math.h>


using namespace std;
using namespace Belle2;

namespace VXDTFMinMaxCollectorTest {

  /** Test class for these new and shiny two-hit-filters. */
  class MinMaxCollectorTest : public ::testing::Test {
  protected:
  };


  /** test what happens if used the wrong way */
  TEST_F(MinMaxCollectorTest, CheckUserHandling)
  {
    // only requests for quantiles 0-0.5 are allowed, for quantile cut q, MinMaxCollector collects quantile-ranges 0 - q & (1-q) - 1.
    EXPECT_ANY_THROW(MinMaxCollector<double>(0.55));
    EXPECT_ANY_THROW(MinMaxCollector<double>(-0.1));
    EXPECT_ANY_THROW(MinMaxCollector<double>(42.));

    double quantileCut = 0.025;
    auto newCollector = MinMaxCollector<double>(0.025);

    // request for quantiles in empty container:
    EXPECT_ANY_THROW(newCollector.getMinMax(0., 1.));

    for (unsigned i = 1 ; i < 501 ; i++) { // fill up to 500
      newCollector.append(gRandom->Uniform(1.));
    }

    EXPECT_ANY_THROW(newCollector.getMinMax(quantileCut + 0.1, 1.)); // first breaks, second is okay
    EXPECT_ANY_THROW(newCollector.getMinMax(0.1, 1. + quantileCut));   // first is okay, second is not

  }



  /** checks if standard filling has any severe issues */
  TEST_F(MinMaxCollectorTest, SomeBasicFillingAndTesting)
  {
    auto newCollector = MinMaxCollector<double>();


    EXPECT_EQ(0, newCollector.size());
    EXPECT_EQ(0, newCollector.totalSize());
    EXPECT_EQ(0, newCollector.sampleSize());
    EXPECT_TRUE(newCollector.empty());

    B2INFO("add first entry");
    newCollector.append(42.);
    newCollector.print();

    EXPECT_EQ(1, newCollector.size());
    EXPECT_EQ(2, newCollector.totalSize());
    EXPECT_EQ(1, newCollector.sampleSize());
    EXPECT_FALSE(newCollector.empty());

    EXPECT_EQ(42., newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(42., newCollector.getMinMax(0.01, 0.99).second);

    B2INFO("add 2nd entry");
    newCollector.append(23.);
    newCollector.print();

    EXPECT_EQ(2, newCollector.size());
    EXPECT_EQ(4, newCollector.totalSize()); // did not grow, old value replaced...
    EXPECT_EQ(2, newCollector.sampleSize());
    EXPECT_EQ(23., newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(42., newCollector.getMinMax(0.01, 0.99).second);

    B2INFO("add 3rd entry");
    newCollector.append(5.);
    newCollector.print();

    EXPECT_EQ(3, newCollector.size());
    EXPECT_EQ(6, newCollector.totalSize()); // did not grow, old value replaced...
    EXPECT_EQ(3, newCollector.sampleSize());
    EXPECT_EQ(5., newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(42., newCollector.getMinMax(0.01, 0.99).second);

    B2INFO("add 47 more entries (sorted add, now 50)");
    for (int i = 1; i < 48; i++) {
      newCollector.append(5. + 0.25 * double(i));
    }
    newCollector.print();

    EXPECT_EQ(9, newCollector.size());
    EXPECT_EQ(18, newCollector.totalSize()); // did not grow, old value replaced...
    EXPECT_EQ(50, newCollector.sampleSize());
    EXPECT_EQ(5., newCollector.getMinMax(0.009, 0.991).first);
    EXPECT_EQ(42., newCollector.getMinMax(0.009, 0.991).second);
    EXPECT_EQ(5.25, newCollector.getMinMax(0.02, 0.98).first);
    EXPECT_EQ(23., newCollector.getMinMax(0.02, 0.98).second);
    EXPECT_NE(16.75, newCollector.getMinMax(0.02, 0.98).second);

    B2INFO("add 50 more entries (sorted add, now 100)");
    for (int i = 1; i < 51; i++) {
      newCollector.append(23. + 0.25 * double(i));
    }
    B2INFO("add 150 more entries (sorted add, now 250)");
    for (int i = 1; i < 151; i++) {
      newCollector.append(3. + 0.3 * double(i));
    }
    newCollector.print(true);
    newCollector.print();

    EXPECT_EQ(10, newCollector.size());
    EXPECT_EQ(unsigned(newCollector.size() * 2), newCollector.totalSize());
    EXPECT_EQ(250, newCollector.sampleSize());
    EXPECT_EQ(3.3, newCollector.getMinMax(0., 1.).first);
    // worst case: through sorted adding, some of the lower quantiles are lost
    EXPECT_EQ(3.9, newCollector.getMinMax(0.009, 0.99).first);
    EXPECT_NE(3.9, newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_NE(4.5, newCollector.getMinMax(0.02, 0.98).first);
    EXPECT_EQ(48., newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(47.4, newCollector.getMinMax(0.01, 0.991).second);
    EXPECT_EQ(46.5, newCollector.getMinMax(0.02, 0.98).second);
  }



  /** fill MinMaxCollector and a std::vector and compare results */
  TEST_F(MinMaxCollectorTest, ComparisonWithClassicApproach)
  {
    auto newCollector = MinMaxCollector<double>(0.03);

    auto vectorCollector = std::vector<double>();
    unsigned vecSize = vectorCollector.size();

    /** helper function for retrieving cutoffs in a vector. */
    auto getIndex = [&](double quantile) -> unsigned int { return (double(vecSize - 1) * quantile + 0.5); };


    EXPECT_EQ(0, newCollector.size());
    EXPECT_EQ(0, newCollector.totalSize());
    EXPECT_EQ(0, newCollector.sampleSize());
    EXPECT_TRUE(newCollector.empty());


    for (unsigned i = 1 ; i < 51 ; i++) { // fill up to 50
      double val = gRandom->Uniform(1.);
      newCollector.append(val);
      vectorCollector.push_back(val);
    }

    newCollector.print();

    std::sort(vectorCollector.begin(), vectorCollector.end());
    vecSize = vectorCollector.size();

    EXPECT_EQ(vectorCollector.size(), newCollector.sampleSize());
    EXPECT_EQ(vectorCollector.front(), newCollector.getMinMax(0., 1.).first);
    EXPECT_EQ(vectorCollector.back(), newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(vectorCollector.at(1), newCollector.getMinMax(0.011, 0.989).first);
    EXPECT_EQ(vectorCollector.at(vecSize - 2), newCollector.getMinMax(0.011, 0.989).second);



    for (unsigned i = 1 ; i < 151 ; i++) { // fill up to 200
      double val = gRandom->Uniform(1.);
      newCollector.append(val);
      vectorCollector.push_back(val);
    }

    newCollector.print();

    std::sort(vectorCollector.begin(), vectorCollector.end());
    vecSize = vectorCollector.size();

    EXPECT_EQ(vectorCollector.size(), newCollector.sampleSize());
    EXPECT_EQ(vectorCollector.front(), newCollector.getMinMax(0., 1.).first);
    EXPECT_EQ(vectorCollector.back(), newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(vectorCollector.at(2), newCollector.getMinMax(0.011, 0.989).first);
    EXPECT_EQ(vectorCollector.at(vecSize - 3), newCollector.getMinMax(0.011, 0.989).second);
    EXPECT_EQ(vectorCollector.at(4), newCollector.getMinMax(0.022, 0.978).first);
    EXPECT_EQ(vectorCollector.at(vecSize - 5), newCollector.getMinMax(0.022, 0.978).second);



    for (unsigned i = 1 ; i < 801 ; i++) { // fill up to 1000
      double val = gRandom->Uniform(1.);
      newCollector.append(val);
      vectorCollector.push_back(val);
    }

    newCollector.print(true);
    newCollector.print();

    std::sort(vectorCollector.begin(), vectorCollector.end());
    vecSize = vectorCollector.size();

    EXPECT_EQ(vectorCollector.size(), newCollector.sampleSize());
    EXPECT_EQ(vectorCollector.front(), newCollector.getMinMax(0., 1.).first);
    EXPECT_EQ(vectorCollector.back(), newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(vectorCollector.at(getIndex(0.01)), newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(vectorCollector.at(getIndex(0.99)), newCollector.getMinMax(0.01, 0.99).second);
    EXPECT_NEAR(vectorCollector.at(getIndex(0.022)), newCollector.getMinMax(0.022, 0.978).first,
                0.001); // requested quantile is near the threshold, therefore no exact results quaranteed
    EXPECT_EQ(vectorCollector.at(getIndex(0.978)), newCollector.getMinMax(0.022, 0.978).second);



    for (unsigned i = 1 ; i < 9001 ; i++) { // fill up to 10000
      double val = gRandom->Uniform(1.);
      newCollector.append(val);
      vectorCollector.push_back(val);
    }

    newCollector.print();

    std::sort(vectorCollector.begin(), vectorCollector.end());
    vecSize = vectorCollector.size();

    EXPECT_EQ(vectorCollector.size(), newCollector.sampleSize());
    EXPECT_EQ(vectorCollector.front(), newCollector.getMinMax(0., 1.).first);
    EXPECT_EQ(vectorCollector.back(), newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(vectorCollector.at(getIndex(0.01)), newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(vectorCollector.at(getIndex(0.99)), newCollector.getMinMax(0.01, 0.99).second);
    EXPECT_NEAR(vectorCollector.at(getIndex(0.022)), newCollector.getMinMax(0.022, 0.978).first, 0.001);
    EXPECT_EQ(vectorCollector.at(getIndex(0.978)), newCollector.getMinMax(0.022, 0.978).second);



    // starting now a second collector to be merged afterwards:
    auto secondCollector = MinMaxCollector<double>();
    for (unsigned i = 1 ; i < 10001 ; i++) { // fill up to 10000
      double val = gRandom->Uniform(1.);
      secondCollector.append(val);
      vectorCollector.push_back(val);
    }
    newCollector.merge(secondCollector);

    std::sort(vectorCollector.begin(), vectorCollector.end());
    vecSize = vectorCollector.size();

    EXPECT_EQ(vectorCollector.size(), newCollector.sampleSize());
    EXPECT_EQ(vectorCollector.front(), newCollector.getMinMax(0., 1.).first);
    EXPECT_EQ(vectorCollector.back(), newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(vectorCollector.at(getIndex(0.01)), newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(vectorCollector.at(getIndex(0.99)), newCollector.getMinMax(0.01, 0.99).second);
    EXPECT_EQ(vectorCollector.at(getIndex(0.022)), newCollector.getMinMax(0.022, 0.978).first);
    EXPECT_NEAR(vectorCollector.at(getIndex(0.978)), newCollector.getMinMax(0.022, 0.978).second, 0.0005);



    for (unsigned i = 1 ; i < 30001 ; i++) { // fill up to 50,000
      double val = gRandom->Uniform(1.);
      newCollector.append(val);
      vectorCollector.push_back(val);
    }

    newCollector.print();

    std::sort(vectorCollector.begin(), vectorCollector.end());
    vecSize = vectorCollector.size();

    EXPECT_EQ(vectorCollector.size(), newCollector.sampleSize());
    EXPECT_EQ(vectorCollector.front(), newCollector.getMinMax(0., 1.).first);
    EXPECT_EQ(vectorCollector.back(), newCollector.getMinMax(0., 1.).second);
    EXPECT_EQ(vectorCollector.at(getIndex(0.01)), newCollector.getMinMax(0.01, 0.99).first);
    EXPECT_EQ(vectorCollector.at(getIndex(0.99)), newCollector.getMinMax(0.01, 0.99).second);
    EXPECT_EQ(vectorCollector.at(getIndex(0.022)), newCollector.getMinMax(0.022, 0.978).first);
    EXPECT_NEAR(vectorCollector.at(getIndex(0.978)), newCollector.getMinMax(0.022, 0.978).second, 0.0001);
  }
}

