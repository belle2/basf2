/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vector>
#include <functional>

namespace Belle2 {
  /**
   * A module that measures the execution time and memory usage per event.
   *
   * This module adds a ProfileInfo object to the datastore and records
   * execution time and memory usage in it.
   *
   * Both the virtual memory and the resident set size (rss) of the process
   * is reported. Both quantities are important and have a complementary meaning:
   *
   * virtual size (vsize): The memory region the kernal has reserved for the process
   * in its memory management, included swapped pages (which are not in RAM, but on disk) and
   * half-filled pages.
   * It also includes files which get mapped into the memory region of the process, but are only
   * loaded on-demand in RAM. All libraries loaded are memory-mapped and show up in the vsize.
   *
   * resident set size (rss): The Rss is the memory which is actually occupied by
   * the process in the physical RAM of the machine. It usually is significantly smaller
   * than the vsize. This is the quantity which should actually be used when checking
   * and optimizing the memory consumption of the application.
   *
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

    /** Name for rss image output file.
     *
     *  The name of the file for the rss memory usage plot.
     */
    std::string m_rssOutputFileName;

    static const int k_burnIn = 1;         /**< number of events before the average time measurement is started */
    static const int k_maxPoints = 100;    /**< maximal number of profile points */

    /**
     * An internal struct to store pairs of memory usage and time.
     */
    struct MemTime {
      /**
       * Constructor with initialization of memory usage and time to zero.
       */
      MemTime(unsigned long vm = 0, unsigned long rssm = 0, double t = 0) : virtualMem(vm), rssMem(rssm), time(t) {};
      unsigned long virtualMem;  /**< virtual memory usage */
      unsigned long rssMem;  /**< rss memory usage */
      double time;        /**< execution time */
    };

    /**
     * Signature of the lambda functions, which are used to extract the memory
     * usage from teh MemTime structure.
     * Depending on the type of memory usage (rss, vsize), this lamdba function
     * can contain different statements.
     */
    typedef std::function< unsigned long (MemTime const&) > MemoryExtractLambda;

    /**
     * Lambda expression to return the virtual memory from a MemTime data structure
     */
    const MemoryExtractLambda m_extractVirtualMem = [](MemTime const& m) { return m.virtualMem;};

    /**
     * Lambda expression to return the Rss memory from a MemTime data structure
     */
    const MemoryExtractLambda m_extractRssMem = [](MemTime const& m) { return m.rssMem;};

    /**
     * Stores the memory usage of the application over time in plots.
     * @param name The name used to store the memory plot in the data store
     * @param title The title displayed on the memory plot
     * @param xAxisName Text displayed on the y-axis. Depending on the plotted quantitiy,
     *                  either vsize or rss, this can vary.
     * @param imgOutput If this string is not empty a plot of the memory consumption over
     *                  time is also written to disk using imgOutput as file name (must include
     *                  file extension)
     * @param lmdMemoryExtract Lambda expression to extrat the plotted memory quantity from the MemTime
     *                         data structure. Use either m_extractVirtualMem or m_extractRssMem here
     */
    void storeMemoryGraph(std::string name, std::string title, std::string xAxisName, std::string imgOutput,
                          MemoryExtractLambda lmdMemoryExtract);

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
