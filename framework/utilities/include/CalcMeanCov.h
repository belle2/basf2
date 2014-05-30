/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef FRAMEWORK_UTILITIES_CALCMEANCOV_H
#define FRAMEWORK_UTILITIES_CALCMEANCOV_H

#include <cmath>
#include <array>

namespace Belle2 {

  /** Class to calculate mean and and covariance between a number of
   * parameters on running data without storing the actual values, also for
   * weighted entries.
   *
   * It can store more than one value per entry, e.g. x,y,z values but will
   * not calculate correlations, only mean and standard deviation for all
   * values. Also works with weighted entries.
   *
   * See Philippe Pébay, Formulas for Robust, One-Pass Parallel Computation
   * of Covariances and Arbitrary-Order Statistical Moments, SANDIA REPORT
   * SAND2008-6212
   *
   * @tparam N number of parameters
   * @tparam RealType floating point type to be used
   */
  template<int N = 1, class RealType = double>
  class CalcMeanCov {
    static_assert(N > 0, "Number of parameters, N, must be positive");
  public:
    /** type of float variable to use for calculations and storage */
    typedef RealType value_type;

    /** Clear all values */
    void clear() {
      m_entries = 0;
      m_mean.fill(0);
      m_covariance.fill(0);
    }

    /** Update mean and covariance by adding a new, weighted entry.
     * @param weight weight of the entry
     * @param values values for all parameters. The number of parameters
     * must be equal to N
     */
    template<class... T> void addWeighted(value_type weight, T... values) {
      static_assert(sizeof...(values) == N,
                    "Number of arguments must be equal N");
      m_entries += weight;
      addValue<0>(weight, values...);
    }

    /** Update mean and covariance by adding a new entry.
     * @param values values for all parameters. The number of Parameters
     * must be equal to N
     */
    template<class... T> void add(T... values) {
      addWeighted(1.0, values...);
    }

    /** @name Getters
     * No range check if performed on indicies i and j
     */
    /**@{*/

    /** Return the number of entries */
    value_type getEntries() const { return m_entries; }
    /** Return the mean for parameter i */
    value_type getMean(int i) const { return m_mean[i]; }
    /** Return the covariance between parameters i and j */
    value_type getCovariance(int i, int j) const {
      if (m_entries == 0.0) return 0.0;
      return m_covariance[getIndex(i, j)] / m_entries;
    }
    /** Return the correlation coefficient between parameters i and j */
    value_type getCorrelation(int i, int j) const {
      if (i == j) return 1.0;
      if (m_entries == 0.0) return 0.0;
      return getCovariance(i, j) / (getStddev(i) * getStddev(j));
    }
    /** Return the variance for paramter i */
    value_type getVariance(int i) const { return getCovariance(i, i); }
    /** Return the standard deviation for parameter i */
    value_type getStddev(int i) const { return std::sqrt(getVariance(i)); }
    /** Return the weighted sum values for parameter i */
    value_type getSum(int i) const { return getMean(i) * m_entries; }

    /**@}*/

    /** @name Templated getters
     * These getters are templated and provide compile time range checking
     * for the parameter indices
     */
    /**@{*/

    /** Return the mean for parameter i */
    template <int i = 0> value_type getMean() const {
      static_assert(i >= 0 && i < N, "index i out of range");
      return m_mean[i];
    }
    /** Return the covariance between parameters i and j */
    template <int i, int j> value_type getCovariance() const {
      static_assert(i >= 0 && i < N, "index i out of range");
      static_assert(j >= 0 && j < N, "index j out of range");
      if (m_entries == 0.0) return 0.0;
      return m_covariance[getIndex(i, j)] / m_entries;
    }
    /** Return the correlation coefficient between parameters i and j */
    template <int i, int j> value_type getCorrelation() const {
      if (i == j) return 1.0;
      if (m_entries == 0.0) return 0.0;
      return getCovariance<i, j>() / (getStddev<i>() * getStddev<j>());
    }
    /** Return the variance for paramter i */
    template <int i = 0> value_type getVariance() const {
      return getCovariance<i, i>();
    }
    /** Return the standard deviation for parameter i */
    template <int i = 0> value_type getStddev() const {
      return std::sqrt(getVariance<i>());
    }
    /** Return the weighted sum values for parameter i */
    template <int i = 0> value_type getSum() const {
      return getMean<i>() * m_entries;
    }

    /**@}*/

  private:
    /** Add a single value for parameter i and update mean and covariance.
     * @tparam i index of the parameter
     * @param weight weight of the entry
     * @param x actual value
     * @param values remaining values to be added recursively
     */
    template<int i, class... T>
    void addValue(value_type weight, value_type x, T... values) {
      const value_type delta = (x - m_mean[i]);
      //Update covariance elements
      updateCov<i, i>(weight, delta, x, values...);
      //Update mean value
      m_mean[i] += weight * delta / m_entries;
      //recursively call again for remaining values
      addValue < i + 1 > (weight, values...);
    }
    /** Break recursion of addValue when no parameters are left */
    template<int i> void addValue(value_type) {}

    /** Update covariance between parameters i and j
     * @tparam i first index
     * @tparam j second index
     * @param weight weight of the entry
     * @param delta difference between parameter i and mean of i
     * @param x value of parameter j
     * @param values remaining values with index >j
     */
    template<int i, int j, class... T>
    void updateCov(value_type weight, value_type delta, value_type x,
                   T... values) {
      const value_type delta2 = (x - m_mean[j]);
      m_covariance[getIndex(i, j)] += (m_entries - weight) * weight
                                      / m_entries * delta * delta2;
      updateCov < i, j + 1 > (weight, delta, values...);
    }

    /** Break recursion of updateCov once all parameters are consumed */
    template<int i, int j> void updateCov(value_type, value_type) {}

    /** Access element in triangular matrix including diagonal elements.
     * This function returns the storage index of an element (i,j) in a
     * symmetric matrix including diagonal elements if the elements are
     * stored in a continous array of size n(n+1)/2
     */
    constexpr int getIndex(int i, int j) const {
      return (i < j) ? ((j + 1) * j / 2 + i) : ((i + 1) * i / 2 + j);
    }

    /** Store the sum of weights */
    value_type m_entries {0};
    /** Store the mean values for all parameters */
    std::array <value_type, N> m_mean {{0}};
    /** Store the triangular covariance matrix for all parameters in
     * continous memory. Actual covariance is m_covariance[i,j]/m_entries */
    std::array < value_type, (N * (N + 1) / 2) > m_covariance {{0}};
  };

} //Belle2 namespace
#endif // FRAMEWORK_UTILITIES_CALCMEANCOV_H
