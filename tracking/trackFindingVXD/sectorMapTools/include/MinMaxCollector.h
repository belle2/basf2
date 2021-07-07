/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


// includes - stl:
#include <deque>
#include <string>
#include <utility> // std::pair, std::move

#include <math.h>       /* ceil */

// includes - tf-related stuff:
// includes - general fw stuff:
#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>


namespace Belle2 {


  /** A container for collecting data, where min- and max-quantiles near q(0) and q(1) are to be found.
   *
   * can only approximatly find actual quantiles and is sensitive to sorted data .
   * -> random samples have to be used!
   * Always exact for min and max, but the nearer the requested quantiles to the given quantileCut, the more sensitive it is to sorted data.
   * For random input, the quantiles always stay very near to the real ones (if they are not the correct ones already).
   * Prerequisites for DataType:
   * - has to be one supported by std::numeric_limits
   **/
  template<class DataType>
  class MinMaxCollector {
  protected:
    /** collects smallest values occured so far */
    std::deque<DataType> m_smallestValues;

    /** collects biggest values occured so far */
    std::deque<DataType> m_biggestValues;

    /** counts numbre of values added so far */
    unsigned m_sampleSize;


    /** sets the threshold for storing data. min- and max-container will store the smalles/biggest quantileCut*100 (== %) of the total sample. */
    DataType m_quantileCut;


    /** sets a threshold for warm-up. The higher the value, the more accurate the quantiles will be, but the more overhead for small sample sizes will be too. */
    unsigned m_warmUpThreshold;



    /** add entry to minContainer if it fits in */
    bool addMin(DataType newVal, bool allow2Grow) { return addEntry(newVal, true, allow2Grow); }



    /** add entry to maxContainer if it fits in */
    bool addMax(DataType newVal, bool allow2Grow) { return addEntry(newVal, false, allow2Grow); }



    /** add entry to container if it fits in */
    bool addEntry(DataType newVal, bool isMinContainer, bool allow2Grow)
    {
      if (isMinContainer) {
        if (m_smallestValues.empty() or allow2Grow) { sortIn(newVal, true); return true; }
        if (m_smallestValues.back() > newVal) {
          m_smallestValues.pop_back();
          sortIn(newVal, true);
          return true;
        }
      } else {
        if (m_biggestValues.empty() or allow2Grow) { sortIn(newVal, false); return true; }
        if (m_biggestValues.front() < newVal) {
          m_biggestValues.pop_front();
          sortIn(newVal, false);
          return true;
        }
      }
      return false;
    }



    /** add newVal to appropriate container */
    void sortIn(DataType newVal, bool isMinContainer)
    {
      /** TODO
       *
       * replace deque with vector and implement individual sort functions for smallest- and biggestValues.
       */
      if (isMinContainer) {
        m_smallestValues.push_back(newVal);
        std::sort(m_smallestValues.begin(), m_smallestValues.end());
        return;
      }
      m_biggestValues.push_back(newVal);
      std::sort(m_biggestValues.begin(), m_biggestValues.end());
      return;
    }



    /** the correct access-index for given quantile will be determined */
    unsigned getIndex(DataType aQuantile) const
//  { return floor(double(vecSize-1) * quantile + 0.5); };
    { return unsigned(double(m_sampleSize/*-1*/) * double(aQuantile) + 0.5); }



    /** returns true, if vector is allowed to grow */
    bool checkVectorSize(std::deque<DataType>& container)
    {
      // want to allow growing with some extra margin to prevent issues
      unsigned newCalcThreshold = unsigned(ceil(float(m_sampleSize) * float(m_quantileCut) + ceil(0.01 * float(m_sampleSize))));

      if (newCalcThreshold > container.size() /*+1*/) { return true; }
      return false;
    }

    /** returns true, if the valueContainers were increased in size. */
//  bool checkResize()
//  {
//    float sampleSize = float(m_sampleSize);
//    float nSmallestValues = float(m_smallestValues.size());
//    float nBiggestValues = float(m_biggestValues.size());
//    float quantileCut = float(m_quantileCut);
//
//    float threshold = sampleSize*quantileCut + ceil(0.01*sampleSize);
//    if (threshold < nSmallestValues or threshold < nBiggestValues) return false;
//
//    unsigned newSize = ceil(threshold*1.1 + 0.5); // stretch by further 10%
//    // fill up with dummy values
//    m_smallestValues.resize(newSize, std::numeric_limits<DataType>::max());
//    m_biggestValues.resize(newSize, std::numeric_limits<DataType>::min());
//    return true;
//  }
  public:

    /** exception shall be thrown if value is not between 0-1 and therefore not normalized */
    BELLE2_DEFINE_EXCEPTION(Quantile_out_of_bounds, "The quantileCut (%1%) you gave is illegal (only allowed between 0-1)");


    /** exception shall be thrown if the requested quantiles are not within the ranges collected */
    BELLE2_DEFINE_EXCEPTION(Illegal_quantile,
                            "The quantiles you asked for (%1% and %2%) are not within the collected range of data (0-%3% and %4%-1) to prevent this happening, you have to pass a bigger value for QuantileCut when constructing a MinMaxCollector-Instance!");


    /** exception shall be thrown if value is not between 0-1 and therefore not normalized */
    BELLE2_DEFINE_EXCEPTION(Request_in_empty_Container, "Data of an empty container was requested!");



    /** constructor. quantileCut determines the fraction of the sample to be stored (0-1). warmUpThreshold pays some overhead for sample sizes to get more accuracy (for bigger sample sizes the overhead and the advantage of warmUp vanish (since then the results converge to the real ones anyway). */
    MinMaxCollector(DataType quantileCut = 0.025, unsigned warmUpThreshold = 10) :
      m_sampleSize(0),
      m_quantileCut(quantileCut),
      m_warmUpThreshold(warmUpThreshold)
    {
      if (0 > quantileCut or 0.5 < quantileCut) { throw (Quantile_out_of_bounds() << quantileCut); }
    }



    /** overloaded '<<' stream operator. Print secID to stream by converting it to string */
    friend std::ostream& operator<< (std::ostream& out, const MinMaxCollector& mmCol) { out << mmCol.getName(); return out; }



    /** for given pair of quantiles, the according cuts (min, max) will be returned. */
    std::pair<DataType, DataType> getMinMax(DataType minQuantile = 0., DataType maxQuantile = 1.) const
    {
      if (m_biggestValues.empty() or m_smallestValues.empty()) { throw (Request_in_empty_Container()); }
      if (0 > minQuantile or 1 < minQuantile) { throw (Quantile_out_of_bounds() << minQuantile); }
      if (0 > maxQuantile or 1 < maxQuantile) { throw (Quantile_out_of_bounds() << maxQuantile); }
      if (minQuantile > m_quantileCut or maxQuantile < (1. - m_quantileCut)) {
        throw (Illegal_quantile() << minQuantile << maxQuantile << m_quantileCut << 1. - m_quantileCut);
      }

      unsigned minIndex = getIndex(minQuantile);
//    B2INFO("minIndex: " << minIndex);
      unsigned maxIndex = getIndex(1. - maxQuantile);
//    B2INFO("maxIndex: " << maxIndex);
      unsigned finalMaxIndex = ((int(m_biggestValues.size()) - int(maxIndex) - 1) < 0) ? 0 : m_biggestValues.size() - 1 - maxIndex;

      if (minIndex > (m_smallestValues.size() - 1)) { B2ERROR("minIndex " << minIndex << " calculated for minQuantile " << minQuantile << " bigger than nSmallestValues " << m_smallestValues.size() << "!"); }
      if (finalMaxIndex > (m_biggestValues.size() - 1)) { B2ERROR("maxIndex " << maxIndex << " calculated for maxQuantile " << maxQuantile << " bigger than nBiggestValues " << m_biggestValues.size() << "!"); }
      return {m_smallestValues.at(minIndex), m_biggestValues.at(finalMaxIndex)};
    }



    /** for convenience reasons, pipe to append. */
    void insert(DataType newVal)
    { append(std::move(newVal));}



    /** for convenience reasons, pipe to append. */
    void push_back(DataType newVal)
    { append(std::move(newVal));}



    /** append new value */
    void append(DataType newVal)
    {
      m_sampleSize++;

      if (m_sampleSize < m_warmUpThreshold) { // to shorten warm-up-phase
        m_smallestValues.push_back(newVal);
        std::sort(m_smallestValues.begin(), m_smallestValues.end());
        m_biggestValues.push_back(newVal);
        std::sort(m_biggestValues.begin(), m_biggestValues.end());
        return;
      }

      /*bool wasAdded =*/ addMin(newVal, checkVectorSize(m_smallestValues));
      /*if (wasAdded == false)*/ addMax(newVal, checkVectorSize(m_biggestValues));
    }



    /** fill the stuff of the other one with this one  */
    void merge(const MinMaxCollector<DataType>& other)
    {
      if (other.m_quantileCut != m_quantileCut) {
        B2WARNING("MinMaxCollector::merge: other collector has differing size in quantileCut. If this is not the purpose, this could indicate unintended behavior!");
      }
      if (other.m_quantileCut > m_quantileCut) {
        m_quantileCut = other.m_quantileCut;
      }
      m_smallestValues.insert(m_smallestValues.end(), other.m_smallestValues.begin(), other.m_smallestValues.end());
      std::sort(m_smallestValues.begin(), m_smallestValues.end());
      m_biggestValues.insert(m_biggestValues.end(), other.m_biggestValues.begin(), other.m_biggestValues.end());
      std::sort(m_biggestValues.begin(), m_biggestValues.end());
      m_sampleSize += other.m_sampleSize;
    }



    /** returns the combined size of the containers storing the values */
    unsigned totalSize() const
    { return m_smallestValues.size() + m_biggestValues.size(); }



    /** returns the size (in a sense of roughly collected data) */
    unsigned size() const
    { return m_smallestValues.size() > m_biggestValues.size() ? m_smallestValues.size() : m_biggestValues.size(); }



    /** returns if internal containers are empty */
    bool empty() const { return (m_smallestValues.empty() and m_biggestValues.empty()); }



    /** deletes all values collected so far and resets to constructor-settings. */
    void clear()
    {
      m_sampleSize = 0;
      m_smallestValues.clear();
      m_biggestValues.clear();
    }

    /** returns actual sampleSize */
    unsigned sampleSize() const { return m_sampleSize; }



    /** print an overview of the entries collected. if parameter given is 'true' full print will be done, if false, the values will be sketched with means. */
    void print(bool printFull = false) const
    { B2INFO(getName(printFull)); }



    /** return a string of an overview of the entries collected. if parameter given is 'true' full print will be done, if false, the values will be sketched with means. */
    std::string getName(bool printFull = false) const
    {
      unsigned nSmallest = m_smallestValues.size();
      unsigned nBiggest = m_biggestValues.size();
      using namespace std;
      string out = "MinMaxCollector with sampleSize " + to_string(m_sampleSize) +
                   " and quantileCut " + to_string(m_quantileCut) +
                   " has nSmallestValues/nBiggestValues: " + to_string(nSmallest) +
                   "/"  + to_string(nBiggest) + "\n";

      if (!printFull) out += "The 5 values each describe for the valueContainer pos[0], pos[1], pos[mean], pos[max-1], pos[max]\n";
      out += "SmallestValues: ";
      // only want to print full vector if there are not many entries in it:
      if (printFull or size() < 6) {
        for (DataType entry : m_smallestValues) { out += to_string(entry) + ", "; }
        out += "\n" + string("BiggestValues: ");
        for (DataType entry : m_biggestValues) { out += to_string(entry) + ", "; }
        out += "\n";
      } else {
        DataType smallestTotal = 0, biggestTotal = 0, smallestMean, biggestMean;
        for (DataType entry : m_smallestValues) { smallestTotal += entry; }
        smallestMean = smallestTotal / DataType(nSmallest);
        out += to_string(m_smallestValues.at(0))
               + ", " + to_string(m_smallestValues.at(1))
               + ", mean: " + to_string(smallestMean)
               + ", " + to_string(m_smallestValues.at(nSmallest - 2))
               + ", " + to_string(m_smallestValues.at(nSmallest - 1));
        for (DataType entry : m_biggestValues) { biggestTotal += entry; }
        biggestMean = biggestTotal / DataType(nBiggest);
        out += "\n" + string("BiggestValues: ");
        out += to_string(m_biggestValues.at(0))
               + ", " + to_string(m_biggestValues.at(1))
               + ", mean: " + to_string(biggestMean)
               + ", " + to_string(m_biggestValues.at(nBiggest - 2))
               + ", " + to_string(m_biggestValues.at(nBiggest - 1));
      }
      return out;
    }

  };
} //Belle2 namespace
