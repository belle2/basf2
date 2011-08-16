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
#include <time.h>


namespace Belle2 {

  /** Store information, that is constant during a run, and is needed during the event loop.
   *
   *  This object is supposed to be updated during the beginRun function.
   *  Currently stored information: <br>
   *  LER beam energy, HER beam energy, crossing angle of the beams, and angle between LER beam and solenoid axis. <br>
   *  The latter two are probably constants during the lifetime of the experiment, but maybe changed for design studies. <br>
   *  The Center of Mass energy and the \f$\gamma\f$ factor of the initial resonance in the lab frame can be calculated from the
   *  these variables.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=RunMetaData">Martin Heck</a>
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

    /** Parents setter.
     *
     *  @param parents The vector of parent IDs.
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

    /** ROOT Macro for FileMetaData dictionary.
     */
    ClassDef(FileMetaData, 1);

  }; //class
} // namespace Belle2
#endif // FILEMETADATA
