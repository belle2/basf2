/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PROFILEMODULE_H
#define PROFILEMODULE_H

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {
  /**
   * A module that measures the execution time and memory usage per event.
   *
   * This module adds a ProfileInfo object to the datastore and records
   * execution time and memory usage in it.
   */
  class ProfileModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    ProfileModule();

    /** Initializes the Module.
     *
     * Creates the ProfileInfo objects in the data store and starts the profiling.
     */
    virtual void initialize();

    /** Event profiling.
     *
     * Records the event profiling information.
     */
    virtual void event();

    /** Terminate the Module.
     *
     * Records the final profiling information.
     */
    virtual void terminate();

  private:

    /** Name for image output file.
     *
     *  The name of the file for the memory usage plot.
     */
    std::string m_outputFileName;

    static const int k_burnIn = 1;         /**< number of events before the average time measurement is started */
    static const int k_maxPoints = 100;    /**< maximal number of profile points */

    /**
     * An internal struct to store pairs of memory usage and time.
     */
    struct MemTime {
      /**
       * Constructor with initialization of memory usage and time to zero.
       */
      MemTime(unsigned long m = 0, double t = 0) : mem(m), time(t) {};
      unsigned long mem;  /**< memory usage */
      double time;        /**< execution time */
    };

    double m_timeOffset;      /**< time at module creation */
    MemTime m_initializeInfo; /**< memory usage and time at initialization */
    MemTime m_terminateInfo;  /**< memory usage and time at termination */
    int m_nEvents;            /**< event counter */
    int m_step;               /**< number of events between profile points */
    std::vector<MemTime> m_eventInfo; /**< memory usage and time at individual events */
    MemTime m_startInfo;      /**< memory usage and time at start of event loop + burn in */
    MemTime m_endInfo;        /**< memory usage and time at end of event loop */

  };
}

#endif /* PROFILEMODULE_H_ */
