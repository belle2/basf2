/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <iosfwd>
#include <Rtypes.h>

namespace Belle2 {
  class EventMetaData;

  /**
   * A class that describes the interval of experiments/runs
   * for which an object in the database is valid.
   */
  class IntervalOfValidity final {
  public:

    /**
     * Constructor. By default an empty validity interval is created where low and high experiment numbers are -1.
     * @param experimentLow   lowest experiment number of the validity range, -1 means no bound
     * @param runLow          lowest run number in the experiment with number experimentLow of the validity range, -1 means no bound
     * @param experimentHigh  highest experiment number of the validity range, -1 means no bound
     * @param runHigh         highest run number in the experiment with number experimentHigh of the validity range, -1 means no bound
     */
    IntervalOfValidity(int experimentLow = -1, int runLow = -1, int experimentHigh = -1, int runHigh = -1);

    /**
     * Destructor.
     */
    ~IntervalOfValidity() = default;

    /**
     * Function that checks whether the event is inside the validity interval.
     * @param event   event meta data to be checked, if the run number 0 it's checked whether the whole experiment is inside the validity interval
     * @return             true if the given event is inside the validity interval.
     */
    bool contains(const EventMetaData& event) const;

    /**
     * Function that checks whether the validity interval is empty.
     * @return             true if the validity interval is empty.
     */
    bool empty() const {return ((m_experimentLow < 0) && (m_experimentHigh < 0));};

    /**
     * Check whether two intervals of validity are identical.
     */
    bool operator==(const IntervalOfValidity& other) const
    {
      return (m_experimentLow == other.m_experimentLow) && (m_runLow == other.m_runLow) &&
             (m_experimentHigh == other.m_experimentHigh) && (m_runHigh == other.m_runHigh);
    }

    /**
     * Check whether two intervals of validity are different.
     */
    bool operator!=(const IntervalOfValidity& other) const
    {
      return !(*this == other);
    }

    /**
     * Function that checks the validity interval contains another interval of validity.
     * @param iov   the other validity interval
     * @return      true if the given other interval of validity is inside the validity interval.
     */
    bool contains(const IntervalOfValidity& iov) const {return overlap(iov) == iov;};

    /**
     * Function that checks the validity interval overlaps with another interval of validity.
     * @param iov   the other validity interval
     * @return      true if both intervals of validity have a common range.
     */
    bool overlaps(const IntervalOfValidity& iov) const {return !overlap(iov).empty();};

    /**
     * Function that determines the overlap of the validity interval with another interval of validity.
     * @param iov   the other validity interval
     * @return      the interval that is common to both intervals of validity.
     */
    IntervalOfValidity overlap(const IntervalOfValidity& iov) const;

    /**
     * Remove the overlap between two intervals of validity by shortening one of them. This does not work if an interval would be cut into two.
     * @param iov         the other validity interval
     * @param trimOlder   flag to determine which validity interval should be trimmed based on the comparison of the lower range bound
     * @return            true if the overlap could be removed.
     */
    bool trimOverlap(IntervalOfValidity& iov, bool trimOlder = true);

    /** Input stream operator for reading IoV data from a text file.
     *
     *  @param input The input stream.
     *  @param metaData The InervalOfValidity object.
     */
    friend std::istream& operator>> (std::istream& input, IntervalOfValidity& iov);

    /** Output stream operator for writing IoV data to a text file.
     *
     *  @param output The output stream.
     *  @param metaData The IntervalOfValidity object.
     */
    friend std::ostream& operator<< (std::ostream& output, const IntervalOfValidity& iov);

  private:

    /** Lowest experiment number. 0 means no bound.
     */
    int m_experimentLow;

    /** Lowest run number. 0 means no bound.
     */
    int m_runLow;

    /** Highest experiment number. 0 means no bound.
     */
    int m_experimentHigh;

    /** Highest run number. 0 means no bound.
     */
    int m_runHigh;

    /**
     * Helper function to check whether a given experiment/run number is above or below the lower bound of the interval of validity.
     * @param experiement   the experiment number
     * @param run           the run number
     * @return              0 if the given experiment/run number is equal to the lower bound, -1 if it is below the lower bound, 1 if it is above the lower bound.
     */
    int checkLowerBound(int experiment, int run) const;

    /**
     * Helper function to check whether a given experiment/run number is above or below the upper bound of the interval of validity.
     * @param experiement   the experiment number
     * @param run           the run number
     * @return              0 if the given experiment/run number is equal to the upper bound, -1 if it is below the upper bound, 1 if it is above the upper bound.
     */
    int checkUpperBound(int experiment, int run) const;

    /**
     * Helper function to set the interval to empty if the upper bound is below the lower one.
     */
    void makeValid();

    ClassDefNV(IntervalOfValidity, 2);  /**< describes the interval of experiments/runs for which an object in the database is valid. */
  };
}
