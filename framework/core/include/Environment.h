/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <list>
#include <string>

namespace Belle2 {

  /**
   * The Environment Class.
   *
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
     * Returns a reference to the list of the modules search filepaths.
     *
     * @return A reference to the list of the modules search filepaths.
     */
    const std::list<std::string>& getModuleSearchPaths() const;

    /**
     * Sets the path which points to the data directory of the framework.
     *
     * @param dataPath Path in which the data files for the framework are located.
     */
    void setDataSearchPath(const std::string& dataPath) { m_dataSearchPath = dataPath; };

    /**
     * Sets the path which points to the externals directory of the framework.
     *
     * @param externalsPath Path in which the externals of the framework are located.
     */
    void setExternalsPath(const std::string& externalsPath) { m_externalsPath = externalsPath; };

    /**
     * Returns the path which points to the data directory of the framework.
     *
     * @return The path in which the data files for the framework are located.
     */
    const std::string getDataSearchPath() const { return m_dataSearchPath; };

    /**
     * Returns the path which points to the externals directory of the framework.
     *
     * @return The path in which the externals of the framework are located.
     */
    const std::string getExternalsPath() const { return m_externalsPath; };

    /** Override the number of events in run 1 for EvtMetaGen module. */
    void setNumberEventsOverride(int nevents) { m_numberEventsOverride = nevents; }

    /** Returns number of events in run 1 for EvtMetaGen module, or 0 for no override. */
    int getNumberEventsOverride() const { return m_numberEventsOverride; }

    /** Override input file name for modules */
    void setInputFileOverride(const std::string& name) { m_inputFileOverride = name; }

    /** Return overriden input file name, or "" if none was set */
    const std::string& getInputFileOverride() const { return m_inputFileOverride; }

    /** Override output file name for modules */
    void setOutputFileOverride(const std::string& name) { m_outputFileOverride = name; }

    /** Return overriden output file name, or "" if none was set */
    const std::string& getOutputFileOverride() const { return m_outputFileOverride; }

    /**
     * Sets the number of processes which should be used for the parallel processing.
     * If the value is set to 0, no parallel processing will be used in the event loop.
     *
     * @param number The number of processes used for the parallel processing.
     */
    void setNumberProcesses(int number) { m_numberProcesses = number; }

    /**
     * Returns the number of processors which should be used for the parallel processing.
     *
     * @return  The number of processors used for the parallel processing.
     */
    int getNumberProcesses() const { return m_numberProcesses; }

    /**
     * Sets the steering file content.
     *
     * @param steering The steering file content.
     */
    void setSteering(const std::string& steering) { m_steering = steering; };

    /**
     * Returns the steering file content.
     *
     * @return The steering file content.
     */
    const std::string& getSteering() { return m_steering; };

  private:

    std::string m_dataSearchPath; /**< The path in which the data files, such as geometry files, are stored. */
    std::string m_externalsPath;  /**< The path in which the externals are located. */
    int m_numberProcesses;        /**< The number of processes that should be used for the parallel processing. */
    std::string m_steering;       /**< The content of the steering file. */
    int m_numberEventsOverride; /**< Override number of events in the first run. */
    std::string m_inputFileOverride; /** Override name of input file for input module */
    std::string m_outputFileOverride; /** Override name of output file for output module */

    /**
     * The constructor is hidden to avoid that someone creates an instance of this class.
     */
    Environment();

    /**
     * Disable/Hide the copy constructor.
     */
    Environment(const Environment&);

    /**
     * Disable/Hide the copy assignment operator.
     */
    Environment& operator=(const Environment&);

    /**
     * The Environment destructor.
     * Deletes the Environment.
     */
    ~Environment();

    static Environment* m_instance; /**< Pointer that saves the instance of this class. */

    /**
     * Destroyer class to delete the instance of the Environment class when the program terminates.
     */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        delete Environment::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

} //end of namespace Belle2

#endif /* ENVIRONMENT_H */
