/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <list>
#include <string>
#include <vector>
#include <memory>

namespace Belle2 {
  class Path;

  /**
   * This class stores all environment information required to run the
   * framework, such as module or data filepaths, number of processes to be used
   * in parallel processing etc. Usually, this information is set by the steering file.
   *
   * This class is designed as a singleton.
   */
  class Environment {

  public:

    /**
     * Static method to get a reference to the Environment instance.
     *
     * @return A reference to an instance of this class.
     */
    static Environment& Instance();

    /**
     * Returns a list of file paths searched for module libraries.
     */
    const std::list<std::string>& getModuleSearchPaths() const;

    /**
     * Sets the path which points to the externals directory of the framework.
     *
     * @param externalsPath Path in which the externals of the framework are located.
     */
    void setExternalsPath(const std::string& externalsPath) { m_externalsPath = externalsPath; };

    /**
     * Returns the path which points to the externals directory of the framework.
     *
     * @return externals path, without trailing slash
     */
    const std::string& getExternalsPath() const { return m_externalsPath; };

    /** Override the number of events in run 1 for EventInfoSetter module. */
    void setNumberEventsOverride(unsigned int nevents) { m_numberEventsOverride = nevents; }

    /** Returns number of events in run 1 for EventInfoSetter module, or 0 for no override. */
    unsigned int getNumberEventsOverride() const { return m_numberEventsOverride; }

    /** Override the number sequences (e.g. 23:42,101) defining the entries which are processed for each filename given in inputFilesOverride*/
    void setEntrySequencesOverride(const std::vector<std::string>& sequences) { m_entrySequencesOverride = sequences; }

    /** Returns the number sequences (e.g. 23:42,101) defining the entry which are processed for each filename given in inputFilesOverride*/
    std::vector<std::string> getEntrySequencesOverride() const { return m_entrySequencesOverride; }

    /** Return the number of events, from either input or EventInfoSetter, or -n command line override (if less). */
    unsigned int getNumberOfEvents() const;

    /** Override run and experiment for EventInfoSetter. */
    void setRunExperimentOverride(int run, int experiment) { m_run = run; m_experiment = experiment; }

    /** Get run override, or -1 if unset. */
    int getRunOverride() const { return m_run; }
    /** Get experiment override, or -1 if unset. */
    int getExperimentOverride() const { return m_experiment; }

    /** Set skipNEvents override */
    void setSkipEventsOverride(unsigned int skipEvents) { m_skipNEvents = skipEvents; }
    /** Get skipNEvents override, or 0 if unset. */
    unsigned int getSkipEventsOverride() const { return m_skipNEvents; }

    /** Number of generated events (from EventInfoSetter). */
    unsigned int getNumberOfMCEvents() const { return m_mcEvents; }

    /** Set number of generated events (for EventInfoSetter). */
    void setNumberOfMCEvents(unsigned int n) { m_mcEvents = n; }

    /** Override input file names for modules */
    void setInputFilesOverride(const std::vector<std::string>& names) { m_inputFilesOverride = names; }

    /** Return overriden input file names, or empty vector if none were set */
    const std::vector<std::string>& getInputFilesOverride() const { return m_inputFilesOverride; }

    /** Override output file name for modules */
    void setOutputFileOverride(const std::string& name) { m_outputFileOverride = name; }

    /** Return overriden output file name, or "" if none was set
     *
     * Note that this will remove the current value to avoid reuse.
     * (e.g. subsequent calls will always return "")
     */
    std::string getOutputFileOverride()
    {
      std::string s = m_outputFileOverride;
      m_outputFileOverride = "";
      return s;
    }

    /** Override number of processes to run in parallel.
     *
     * Only values >= 0 will change the result of getNumberProcesses().
     */
    void setNumberProcessesOverride(int nproc) { m_numberProcessesOverride = nproc; }

    /**
     * Sets the number of processes which should be used for the parallel processing.
     * If the value is set to 0, no parallel processing will be used in the event loop.
     *
     * @param number The number of processes used for the parallel processing.
     */
    void setNumberProcesses(int number) { m_numberProcesses = number; }

    /**
     * Returns the number of worker processes which should be used for the parallel processing.
     */
    int getNumberProcesses() const
    {
      if (m_numberProcessesOverride >= 0)
        return m_numberProcessesOverride;
      else
        return m_numberProcesses;
    }

    /**
     * Sets the path to the file where the pickled path is stored
     *
     * @param path The path to the file where the pickled path is stored.
     */
    void setPicklePath(const std::string& path) { m_picklePath = path; }

    /**
     * Returns the path to the file where the pickled path is stored
     *
     * @return  The path to the file where the pickled path is stored.
     */
    std::string getPicklePath() const { return m_picklePath; }

    /**
     * Sets the steering file content.
     */
    void setSteering(const std::string& steering) { m_steering = steering; };

    /**
     * Returns the steering file content.
     */
    const std::string& getSteering() const { return m_steering; };

    /** Wether to generate DOT files with data store inputs/outputs of each module. */
    void setVisualizeDataFlow(bool on) { m_visualizeDataFlow = on; }

    /** Wether to generate DOT files with data store inputs/outputs of each module. */
    bool getVisualizeDataFlow() const { return m_visualizeDataFlow; }

    /** Disable collection of statistics during event processing. */
    void setNoStats(bool noStats) { m_noStats = noStats; }

    /** Disable collection of statistics during event processing. */
    bool getNoStats() const { return m_noStats; }
    /** Read steering file, but do not start any actually start any event processing. Prints information on input/output files and number of events that that would be used during normal execution. */
    void setDryRun(bool dryRun) { m_dryRun = dryRun; }
    /** Read steering file, but do not start any actually start any event processing. Prints information on input/output files and number of events that that would be used during normal execution. */
    bool getDryRun() const { return m_dryRun; }

    /** Set info from path executed by the framework. */
    void setJobInformation(const std::shared_ptr<Path>& path);

    /** Print information on input/output files in current steering file, used by --dry-run.
     *
     *  Function only relies on information available during module construction,
     *  but requires that Framework::process() has been called to set a path.
     **/
    void printJobInformation() const;

    /** Set the name of a module to be profiled */
    void setProfileModuleName(const std::string& name) { m_profileModuleName = name; }

    /** Return the name of the module to be profiled, empty if no profiling was requested */
    std::string getProfileModuleName() const { return m_profileModuleName; }

    /** Override global log level if != LogConfig::c_Default. */
    void setLogLevelOverride(int level) { m_logLevelOverride = level; }

    /** Get log level override. */
    int getLogLevelOverride() const { return m_logLevelOverride; }

    /** Set list of streaming objects */
    void setStreamingObjects(const std::vector<std::string>& strobjs) { m_streamingObjects = strobjs; }

    /** Get list of streaming objects */
    const std::vector<std::string>& getStreamingObjects() const { return m_streamingObjects; }

  private:

    std::string m_externalsPath;  /**< The path in which the externals are located. */
    int m_numberProcesses;        /**< The number of worker processes that should be used for the parallel processing. */
    std::string m_steering;       /**< The content of the steering file. */
    unsigned int m_numberEventsOverride;   /**< Override number of events in the first run. */
    std::vector<std::string> m_inputFilesOverride; /**< Override input file names for input modules */
    std::vector<std::string>
    m_entrySequencesOverride; /**< A number sequence (e.g. 23:42,101) defining the entries which are processed for each input file in m_inputFilesOverride.*/
    std::string m_outputFileOverride; /**< Override name of output file for output module */
    int m_numberProcessesOverride; /**< Override m_numberProcesses if >= 0 */
    int m_logLevelOverride; /**< Override global log level if != LogConfig::c_Default. */
    bool m_visualizeDataFlow; /**< Wether to generate DOT files with data store inputs/outputs of each module. */
    bool m_noStats; /**< Disable collection of statistics during event processing. Useful for very high-rate applications. */
    bool m_dryRun; /**< Read steering file, but do not start any actually start any event processing. Prints information on input/output files that that would be used during normal execution. */
    std::string m_jobInfoOutput; /**< Output for printJobInformation(), generated by setJobInformation(). */
    std::string m_profileModuleName; /**< Name of the module which should be profiled, empty if no profiling requested */
    std::string m_picklePath; /**< Path to the file where the pickled path is stored */
    std::vector<std::string> m_streamingObjects;  /**< objects to be streamed in Tx module (all if empty) */
    unsigned int m_mcEvents; /**< counter for number of generated events. */
    int m_run; /**< override run for EventInfoSetter. */
    int m_experiment; /**< override experiment for EventInfoSetter. */
    unsigned int m_skipNEvents; /**< override skipNEvents for EventInfoSetter/RootInput. */

    /**
     *  Set up environment from standard BELLE2_ environment variables.
     *
     *  Ends with B2FATAL if something goes wrong.
     */
    Environment();

    /**
     * Disable/Hide the copy constructor.
     */
    Environment(const Environment&) = delete;

    /**
     * Disable/Hide the copy assignment operator.
     */
    Environment& operator=(const Environment&) = delete;

    /**
     * The Environment destructor.
     */
    ~Environment();

  };

} //end of namespace Belle2
