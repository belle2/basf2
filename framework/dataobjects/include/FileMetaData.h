/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2013 - Belle II Collaboration                        *
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

#include <iosfwd>
#include <vector>

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
    int getId() const {return m_id;}

    /** Grid unique identifier getter.
     */
    std::string getGuid() const {return m_guid;}

    /** Logical file name getter.
     */
    std::string getLfn() const {return m_lfn;}

    /** Number of events getter.
     */
    unsigned long getEvents() const {return m_events;}

    /** Lowest experiment number getter.
     */
    unsigned long getExperimentLow() const {return m_experimentLow;}

    /** Lowest run number getter.
     */
    unsigned long getRunLow() const {return m_runLow;}

    /** Lowest event number in lowest run getter.
     */
    unsigned long getEventLow() const {return m_eventLow;}

    /** Highest experiment number getter.
     */
    unsigned long getExperimentHigh() const {return m_experimentHigh;}

    /** Highest run number getter.
     */
    unsigned long getRunHigh() const {return m_runHigh;}

    /** Highest event number in highest run getter.
     */
    unsigned long getEventHigh() const {return m_eventHigh;}

    /** Check whether the given event is in the covered range of events.
     *
     *  @param experiment The experiment number of the event.
     *  @param run The run number of the event.
     *  @param event The event number of the event.
     */
    bool containsEvent(unsigned long experiment, unsigned long run, unsigned long event) const;

    /** Get number of parent files.
     */
    int getNParents() const {return m_parents.size();}

    /** Get unique identifier of parent file.
     *
     *  @param iParent The number of the parent file.
     */
    int getParent(int iParent) const {return m_parents[iParent];}

    /** Software release version getter.
     */
    std::string getRelease() const {return m_release;}

    /** File creation date and time getter.
     */
    time_t getTimeStamp() const {return m_timeStamp;}

    /** Site where the file was created getter.
     */
    std::string getSite() const {return m_site;}

    /** User who created the file getter.
     */
    std::string getUser() const {return m_user;}

    /** Unique identifier of log file getter.
     */
    int getLogFile() const {return m_logFile;}

    /** Random seed getter.
     */
    unsigned int getRandomSeed() const {return m_randomSeed;}

    /** Initital random number generator object getter.
     */
    const TRandom3& getRandom() const {return m_random;}

    /** Steering file content getter.
     */
    std::string getSteering() const {return m_steering;}

    /** Number of events setter.
     *
     *  @param events The number of events.
     */
    void setEvents(unsigned long events) {m_events = events;}

    /** Lowest experiment, run and event number setter.
     *
     *  @param experiment The lowest experiment number.
     *  @param run The lowest run number.
     *  @param event The lowest event number of the lowest run.
     */
    void setLow(unsigned long experiment, unsigned long run, unsigned long event) {m_experimentLow = experiment; m_runLow = run; m_eventLow = event;}

    /** Highest experiment, run and event number setter.
     *
     *  @param experiment The highest experiment number.
     *  @param run The highest run number.
     *  @param event The highest event number of the highest run.
     */
    void setHigh(unsigned long experiment, unsigned long run, unsigned long event) {m_experimentHigh = experiment; m_runHigh = run; m_eventHigh = event;}

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

    /** Print the content of the meta data object.
     *
     *  @param option Use "all" to print everything, except steering file. Use "steering" for printing steering file.
     */
    virtual void Print(Option_t* option = "") const;

    /** Input stream operator for reading file meta data from the file catalog in xml format.
     *
     *  @param input The input stream.
     *  @param metaData The FileMetaData object.
     */
    friend std::istream& operator>> (std::istream& input, FileMetaData& metaData);

    /** Output stream operator for writing file meta data to the file catalog in xml format.
     *
     *  @param output The output stream.
     *  @param metaData The FileMetaData object.
     */
    friend std::ostream& operator<< (std::ostream& output, const FileMetaData& metaData);

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

    /** Lowest experiment number.
     */
    unsigned long m_experimentLow;

    /** Lowest run number.
     */
    unsigned long m_runLow;

    /** Lowest event number in lowest run.
     */
    unsigned long m_eventLow;

    /** Highest experiment number.
     */
    unsigned long m_experimentHigh;

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

    ClassDef(FileMetaData, 3); /**< Metadata information about a file. */

  }; //class

} // namespace Belle2
#endif // FILEMETADATA
