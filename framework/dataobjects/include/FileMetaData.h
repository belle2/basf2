/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef FILEMETADATA_H
#define FILEMETADATA_H

#include <TObject.h>
#include <TRandom3.h>
#include <time.h>


namespace Belle2 {

  /** Metadata information about a file
   */
  class FileMetaData : public TObject {
  public:


    /** Constructor.
     */
    FileMetaData();

    /** Unique identifier getter.
     */
    int getId() {return m_id;}

    /** Grid unique identifier getter.
     */
    std::string getGuid() {return m_guid;}

    /** Logical file name getter.
     */
    std::string getLfn() {return m_lfn;}

    /** Number of events getter.
     */
    unsigned long getEvents() {return m_events;}

    /** Experiment number getter.
     */
    unsigned long getExperiment() {return m_experiment;}

    /** Lowest run number getter.
     */
    unsigned long getRunLow() {return m_runLow;}

    /** Lowest event number in lowest run getter.
     */
    unsigned long getEventLow() {return m_eventLow;}

    /** Highest run number getter.
     */
    unsigned long getRunHigh() {return m_runHigh;}

    /** Highest event number in highest run getter.
     */
    unsigned long getEventHigh() {return m_eventHigh;}

    /** Get number of parent files.
     */
    int getNParents() {return m_parents.size();}

    /** Get unique identifier of parent file.
     *
     *  @param iParent The number of the parent file.
     */
    int getParent(int iParent) {return m_parents[iParent];}

    /** Software release version getter.
     */
    std::string getRelease() {return m_release;}

    /** File creation date and time getter.
     */
    time_t getTimeStamp() {return m_timeStamp;}

    /** Site where the file was created getter.
     */
    std::string getSite() {return m_site;}

    /** User who created the file getter.
     */
    std::string getUser() {return m_user;}

    /** Unique identifier of log file getter.
     */
    int getLogFile() {return m_logFile;}

    /** Random seed getter.
     */
    unsigned int getRandomSeed() {return m_randomSeed;}

    /** Initital random number generator object getter.
     */
    const TRandom3 getRandom() {return m_random;}

    /** Steering file content getter.
     */
    std::string getSteering() {return m_steering;}

    /** Number of events setter.
     *
     *  @param events The number of events.
     */
    void setEvents(unsigned long events) {m_events = events;}

    /** Experiment number setter.
     *
     *  @param experiment The experiment number.
     */
    void setExperiment(unsigned long experiment) {m_experiment = experiment;}

    /** Lowest run and event number setter.
     *
     *  @param run The lowest run number.
     *  @param event The lowest event number of the lowest run.
     */
    void setLow(unsigned long run, unsigned long event) {m_runLow = run; m_eventLow = event;}

    /** Highest run and event number setter.
     *
     *  @param run The highest run number.
     *  @param event The highest event number of the highest run.
     */
    void setHigh(unsigned long run, unsigned long event) {m_runHigh = run; m_eventHigh = event;}

    /** Parents setter.
     *
     *  @param parents The vector of parent IDs.
     */
    void setParents(const std::vector<int>& parents) {m_parents = parents;}

    /** Creation data setter.
     *
     *  @param release The software release.
     *  @param timeStamp The creation date and time.
     *  @param site The site where the file was created.
     *  @param user The user who created the file.
     */
    void setCreationData(const std::string& release, time_t timeStamp, const std::string& site, const std::string& user)
    {m_release = release; m_timeStamp = timeStamp; m_site = site; m_user = user;}

    /** Setter for IDs.
      *
      *  @param id The unique identifier.
      *  @param guid The grid unique identifier.
      *  @param lfn The logical file name.
      *  @param logFile The ID of the log file.
      */
    void setIds(int id, const std::string& guid, const std::string& lfn, int logFile = 0)
    {m_id = id; m_guid = guid; m_lfn = lfn; m_logFile = logFile;}

    /** Random seed and number generator setter.
     *
     *  @param seed The random seed.
     *  @param random The random number generator object.
     */
    void setRandom(unsigned int seed, const TRandom3& random) {m_randomSeed = seed; m_random = random;}

    /** Steering file content setter.
     *
     *  @param steering The content of the steering file.
     */
    void setSteering(const std::string& steering) {m_steering = steering;}

    /**
     * Exposes methods of the FileMateData class to Python.
     */
    static void exposePythonAPI();


  private:

    /** Unique identifier.
     */
    int m_id;

    /** Grid unique identifier.
     */
    std::string m_guid;

    /** Logical file name.
     */
    std::string m_lfn;

    /** Number of events.
     */
    unsigned long m_events;

    /** Experiment number.
     */
    unsigned long m_experiment;

    /** Lowest run number.
     */
    unsigned long m_runLow;

    /** Lowest event number in lowest run.
     */
    unsigned long m_eventLow;

    /** Highest run number.
     */
    unsigned long m_runHigh;

    /** Highest event number in highest run.
     */
    unsigned long m_eventHigh;

    /** Unique identifiers of parent files.
     */
    std::vector<int> m_parents;

    /** Software release version.
     */
    std::string m_release;

    /** File creation date and time.
     */
    time_t m_timeStamp;

    /** Site where the file was created.
     */
    std::string m_site;

    /** User who created the file.
     */
    std::string m_user;

    /** Unique identifier of log file.
     */
    int m_logFile;

    /** The random seed.
     */
    unsigned int m_randomSeed;

    /** The initial random number generator object.
     */
    TRandom3 m_random;

    /** The steering file content.
     */
    std::string m_steering;

    /** ROOT Macro for FileMetaData dictionary.
     */
    ClassDef(FileMetaData, 2);

  }; //class
} // namespace Belle2
#endif // FILEMETADATA
