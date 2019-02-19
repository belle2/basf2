/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <string>
#include <vector>
#include <map>
#include <limits>       // std::numeric_limits


namespace Belle2 {

  /** allows to set outer and inner secID. */
  class SecIDPair {
  public:
    unsigned outer; /**< id of outer sector. */
    unsigned inner; /**< id of inner sector. */

    /** constructor - resets all values to outer = std::numeric_limits< unsigned>::max();*/
    SecIDPair() { reset(); }

    /** sets all values to outer = std::numeric_limits< unsigned>::max();*/
    void reset()
    {
      outer = std::numeric_limits<unsigned>::max();
      inner = std::numeric_limits<unsigned>::max();
    }

    /** checks if any value is still not set and returns false if that is the case.
     * (only completely set values in the dataSet are valid). */
    bool isValid()
    {
      return ((outer == std::numeric_limits<unsigned>::max())
              or (inner == std::numeric_limits<unsigned>::max()))
             ? false : true;
    }
  };



  /** allows to set outer, center and inner secID. */
  class SecIDTriplet {
  public:
    unsigned outer; /**< id of outer sector. */
    unsigned center; /**< id of center sector. */
    unsigned inner; /**< id of inner sector. */

    /** constructor - resets all values to outer = std::numeric_limits< unsigned>::max();*/
    SecIDTriplet() { reset(); }

    /** sets all values to outer = std::numeric_limits< unsigned>::max();*/
    void reset()
    {
      outer = std::numeric_limits<unsigned>::max();
      center = std::numeric_limits<unsigned>::max();
      inner = std::numeric_limits<unsigned>::max();
    }

    /** checks if any value is still not set and returns false if that is the case.
     * (only completely set values in the dataSet are valid). */
    bool isValid()
    {
      return ((outer == std::numeric_limits<unsigned>::max())
              or (center == std::numeric_limits<unsigned>::max())
              or (inner == std::numeric_limits<unsigned>::max()))
             ? false : true;
    }
  };



  /** allows to set outer, outerCenter, innerCenter and inner secID. */
  class SecIDQuadruplet {
  public:
    unsigned outer; /**< id of outer sector. */
    unsigned outerCenter; /**< id of outer-center sector. */
    unsigned innerCenter; /**< id of inner-center sector. */
    unsigned inner; /**< id of inner sector. */

    /** constructor - resets all values to outer = std::numeric_limits< unsigned>::max();*/
    SecIDQuadruplet() { reset(); }

    /** sets all values to outer = std::numeric_limits< unsigned>::max();*/
    void reset()
    {
      outer = std::numeric_limits<unsigned>::max();
      outerCenter = std::numeric_limits<unsigned>::max();
      innerCenter = std::numeric_limits<unsigned>::max();
      inner = std::numeric_limits<unsigned>::max();
    }

    /** checks if any value is still not set and returns false if that is the case.
     * (only completely set values in the dataSet are valid). */
    bool isValid()
    {
      return ((outer == std::numeric_limits<unsigned>::max())
              or (outerCenter == std::numeric_limits<unsigned>::max())
              or (innerCenter == std::numeric_limits<unsigned>::max())
              or (inner == std::numeric_limits<unsigned>::max()))
             ? false : true;
    }
  };



  /** contains the relevant information needed for filling a TTree containing train-data for the secMap. */
  template <class SecIDSetType>
  class FilterValueDataSet {
  protected:
    /** map containing a value for each key==name of filter. */
    std::map<std::string, double> m_values;

  public:

    /** number of the experiment this dataset is taken from. */
    unsigned expNo;

    /** number of the run this dataset is taken from. */
    unsigned runNo;

    /** number of the event this dataset is taken from. */
    unsigned evtNo;

    /** number of the reference track this dataset is taken from (its ID in the datastore). */
    unsigned trackNo;

    /** Pdg given by reference track. */
    int pdg;

    /** contains the secIDs. */
    SecIDSetType secIDs;


/////////////////////////////////////////////////////////////////////////// member functions:

    /** constructor, expects to get a vector of names for filters. */
    explicit FilterValueDataSet(std::vector<std::string> filterNames) :
      expNo(std::numeric_limits<unsigned>::max()),
      runNo(std::numeric_limits<unsigned>::max()),
      evtNo(std::numeric_limits<unsigned>::max()),
      trackNo(std::numeric_limits<unsigned>::max()),
      pdg(std::numeric_limits<int>::max())
    {
      for (auto name : filterNames) {
        m_values.insert({name, std::numeric_limits<double>::max()});
        B2DEBUG(60, "FilterValueDataSet::constructor: filterName " << name << " has been added.");
      }
      B2DEBUG(50, "FilterValueDataSet::constructor: " << m_values.size() << " filters have been added.");
    }

    /** for given filterName a value is set. if filterName does not exist, an error is produced and nothing gets stored. */
    void setValueOfFilter(std::string filterName, double value)
    {
      auto it = m_values.find(filterName);
      if (it == m_values.end()) {
        B2ERROR("FilterValueDataSet::setValueOfFilter: filterName " << filterName
                << "is not known, value " << value
                << " will NOT be added!");
        return;
      }
      it->second = value;
    }

    /** returns pointer to value of given filterName, nullptr if filterName has not been found. */
    double* getValuePtr(std::string filterName)
    {
      double* valuePtr = nullptr;
      auto it = m_values.find(filterName);
      if (it != m_values.end()) {
        valuePtr = &it->second;
      } else { B2WARNING("filter could not be found, returning nullptr instead!"); }
      return valuePtr;
    }

    /** resets all values stored to std::numeric_limits< double>::max(). and all FilterIDs to std::numeric_limits< unsigned>::max() */
    void reset()
    {
      expNo = std::numeric_limits<unsigned>::max();
      runNo = std::numeric_limits<unsigned>::max();
      evtNo = std::numeric_limits<unsigned>::max();
      trackNo = std::numeric_limits<unsigned>::max();
      pdg = std::numeric_limits<int>::max();
      secIDs.reset();
      for (auto& entry : m_values) {
        entry.second = std::numeric_limits<double>::max();
      }
    }

    /** checks if any value is still not set and returns false if that is the case.
     * (only completely set values in the dataSet are valid). */
    bool isValid()
    {
      if (expNo == std::numeric_limits<unsigned>::max()) return false;
      if (runNo == std::numeric_limits<unsigned>::max()) return false;
      if (evtNo == std::numeric_limits<unsigned>::max()) return false;
      if (trackNo == std::numeric_limits<unsigned>::max()) return false;
      if (pdg == std::numeric_limits<int>::max()) return false;
      for (auto& entry : m_values) {
        if (entry.second == std::numeric_limits<double>::max()) return false;
      }
      return secIDs.isValid();
    }
  };

}

