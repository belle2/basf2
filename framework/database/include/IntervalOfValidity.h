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

#include <TObject.h>


namespace Belle2 {
  class EventMetaData;

  /**
   * A class that describes the interval of experiments/runs
   * for which an object in the database is valid.
   */
  class IntervalOfValidity {
  public:

    /**
     * Constructor. By default an empty validity interval is created where low and high experiment numbers are 0.
     * @param experimentLow   lowest experiment number of the validity range, 0 means no bound
     * @param runLow          lowest run number in the experiment with number experimentLow of the validity range, 0 means no bound
     * @param experimentHigh  highest experiment number of the validity range, 0 means no bound
     * @param runHigh         highest run number in the experiment with number experimentHigh of the validity range, 0 means no bound
     */
    IntervalOfValidity(unsigned long experimentLow = 0, unsigned long runLow = 0, unsigned long experimentHigh = 0,
                       unsigned long runHigh = 0);

    /**
     * Destructor.
     */
    virtual ~IntervalOfValidity() {};

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
    bool empty() const {return (!m_experimentLow && !m_experimentHigh);};

    /**
     * Check whether two intervals of validity are identical.
     */
    virtual bool operator==(const IntervalOfValidity& other)
    {
      return (m_experimentLow == other.m_experimentLow) && (m_runLow == other.m_runLow) &&
             (m_experimentHigh == other.m_experimentHigh) && (m_runHigh == other.m_runHigh);
    }

    /**
     * Check whether two intervals of validity are different.
     */
    virtual bool operator!=(const IntervalOfValidity& other)
    {
      return !(*this == other);
    }

  private:

    /** Lowest experiment number. 0 means no bound.
     */
    unsigned long m_experimentLow;

    /** Lowest run number. 0 means no bound.
     */
    unsigned long m_runLow;

    /** Highest experiment number. 0 means no bound.
     */
    unsigned long m_experimentHigh;

    /** Highest run number. 0 means no bound.
     */
    unsigned long m_runHigh;

    ClassDef(IntervalOfValidity, 1);  /**< describes the interval of experiments/runs for which an object in the database is valid. */
  };
}
