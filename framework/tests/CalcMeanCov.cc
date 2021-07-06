/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/TestHelpers.h>
#include <framework/utilities/CalcMeanCov.h>
#include <TRandom.h>
#include <TMath.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** Check if online mean/sigma agree with standard algorithm when creating
   * normal distributed values. */
  TEST(CalcMeanCov, Normal)
  {
    constexpr int N = 10000;
    std::vector<double> values(N, 0);
    CalcMeanCov<> meancov;

    for (double gmean : { -5.0, 0.0, 1.0, 3.5, 150.}) {
      for (double gsigma : {0.1, 1.0, 4.2, 25.}) {
        double sum1(0), sum2(0), sum3(0);
        for (int i = 0; i < N; ++i) {
          const double x = gRandom->Gaus(gmean, gsigma);
          sum1 += x;
          values[i] = x;
          meancov.add(x);
        }
        const double mean = sum1 / N;
        for (auto x : values) {
          const double d = x - mean;
          sum2 += d * d;
          sum3 += d;
        }
        const double variance = (sum2 - sum3 * sum3 / N) / N;
        const double sigma = std::sqrt(variance);

        EXPECT_FLOAT_EQ(N, meancov.getEntries());
        EXPECT_FLOAT_EQ(mean, meancov.getMean());
        EXPECT_FLOAT_EQ(sigma, meancov.getStddev());
        EXPECT_FLOAT_EQ(sum1, meancov.getSum());
        meancov.clear();
      }
    }
  }

  /** Test that we can reproduce discrete, Uniform distribution.
   * This distribution is easy to calculate analytical and can also be filled
   * exactly using only unweighted entries.
   */
  TEST(CalcMeanCov, ManyUniform)
  {
    CalcMeanCov<3> meancov;
    constexpr int N = 10001;
    //Let's use a districte, uniform distribution because sum,
    //mean and variance are very easy to calculate
    //Result should not depend on the order of entries and a constant offset
    //should only influence mean and sum, not the variance/standard deviation
    for (int i = 0; i < N; ++i) {
      meancov.add(i + 1, i + 13.5, N - i);
    }
    const double sum1 = (N * N + N) / 2.;
    const double sum2 = sum1 + 12.5 * N;
    const double mean1 = (N + 1.0) / 2;
    const double mean2 = (13.5 + N + 12.5) / 2;
    const double varia = (N * N - 1.) / 12;
    const double sigma = std::sqrt(varia);

    EXPECT_DOUBLE_EQ(N, meancov.getEntries());
    EXPECT_DOUBLE_EQ(mean1, meancov.getMean<0>());
    EXPECT_DOUBLE_EQ(mean2, meancov.getMean<1>());
    EXPECT_DOUBLE_EQ(mean1, meancov.getMean<2>());
    EXPECT_DOUBLE_EQ(sigma, meancov.getStddev<0>());
    EXPECT_DOUBLE_EQ(sigma, meancov.getStddev<1>());
    EXPECT_DOUBLE_EQ(sigma, meancov.getStddev<2>());
    EXPECT_DOUBLE_EQ(sum1, meancov.getSum<0>());
    EXPECT_DOUBLE_EQ(sum2, meancov.getSum<1>());
    EXPECT_DOUBLE_EQ(sum1, meancov.getSum<2>());
    //Check that correlation is correct
    EXPECT_DOUBLE_EQ(+1, (meancov.getCorrelation<0, 1>()));
    EXPECT_DOUBLE_EQ(-1, (meancov.getCorrelation<0, 2>()));
    EXPECT_DOUBLE_EQ(-1, (meancov.getCorrelation<1, 2>()));
    //Check that the covariance matrix is symmetric
    EXPECT_EQ((meancov.getCovariance<1, 0>()), (meancov.getCovariance<0, 1>()));
    EXPECT_EQ((meancov.getCovariance<2, 0>()), (meancov.getCovariance<0, 2>()));
    EXPECT_EQ((meancov.getCovariance<2, 1>()), (meancov.getCovariance<1, 2>()));
  }

  /** Test that we can reproduce the mean/variance of the Binomial distribution.
   * Binomial is well suited as it is discrete defined on a limited range so we
   * can reproduce the exact values without any cut-off.
   */
  TEST(CalcMeanCov, Binomial)
  {
    CalcMeanCov<> meancov;
    for (double n : {10, 13, 25}) {
      for (double p : {0.1, 0.5, 0.75}) {
        for (int k = 0; k <= n; ++k) {
          const double b = TMath::Binomial(n, k) * std::pow(p, k) * std::pow(1 - p, n - k);
          meancov.addWeighted(b, k);
        }
        EXPECT_DOUBLE_EQ(1.0, meancov.getEntries()) << "n: " << n << ", p: " << p;
        EXPECT_DOUBLE_EQ(n * p, meancov.getMean()) << "n: " << n << ", p: " << p;
        EXPECT_DOUBLE_EQ(n * p * (1 - p), meancov.getVariance()) << "n: " << n << ", p: " << p;
        meancov.clear();
      }
    }
  }

  /** Test that the templated and non-templated methods behave in the same way */
  TEST(CalcMeanCov, TemplateVsArray)
  {
    CalcMeanCov<3> templated;
    CalcMeanCov<3> arrayed;
    double values[3];
    for (int i = 0; i < 10000; ++i) {
      const double weight = gRandom->Uniform(0, 10);
      values[0] = gRandom->Gaus(0, 1);
      values[1] = gRandom->Gaus(values[0], 1);
      values[2] = gRandom->Uniform(-5, 5);
      templated.addWeighted(weight, values[0], values[1], values[2]);
      arrayed.addWeightedArray(weight, values);

      ASSERT_EQ(templated.getEntries(), arrayed.getEntries());
      ASSERT_EQ(templated.getSum<0>(), arrayed.getSum(0));
      ASSERT_EQ(templated.getSum<1>(), arrayed.getSum(1));
      ASSERT_EQ(templated.getSum<2>(), arrayed.getSum(2));
      ASSERT_EQ(templated.getMean<0>(), arrayed.getMean(0));
      ASSERT_EQ(templated.getMean<1>(), arrayed.getMean(1));
      ASSERT_EQ(templated.getMean<2>(), arrayed.getMean(2));
      ASSERT_EQ(templated.getStddev<0>(), arrayed.getStddev(0));
      ASSERT_EQ(templated.getStddev<1>(), arrayed.getStddev(1));
      ASSERT_EQ(templated.getStddev<2>(), arrayed.getStddev(2));
      ASSERT_EQ((templated.getCovariance<0, 1>()), arrayed.getCovariance(0, 1));
      ASSERT_EQ((templated.getCovariance<0, 2>()), arrayed.getCovariance(0, 2));
      ASSERT_EQ((templated.getCovariance<1, 2>()), arrayed.getCovariance(1, 2));
    }
  }

  /** Check result of adding two CalcMeanCov sets yields same results as doing it in one set. */
  TEST(CalcMeanCov, AddingTwoSets)
  {
    constexpr int N = 10000;
    CalcMeanCov<> meancov; //adding one by one
    CalcMeanCov<> sum;
    CalcMeanCov<> otherhalf; //added to sum

    for (double gmean : { -5.0, 0.0, 1.0, 3.5, 150.}) {
      for (double gsigma : {0.1, 1.0, 4.2, 25.}) {
        for (int i = 0; i < N; ++i) {
          const double x = gRandom->Gaus(gmean, gsigma);
          meancov.add(x);
          if (i % 2 == 0)
            sum.add(x);
          else
            otherhalf.add(x);
        }
        sum.add(otherhalf);

        EXPECT_FLOAT_EQ(sum.getEntries(), meancov.getEntries());
        EXPECT_FLOAT_EQ(sum.getMean(), meancov.getMean());
        EXPECT_FLOAT_EQ(sum.getStddev(), meancov.getStddev());
        EXPECT_FLOAT_EQ(sum.getSum(), meancov.getSum());
        meancov.clear();
        sum.clear();
        otherhalf.clear();
      }
    }
  }
}  // namespace
