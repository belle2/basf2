/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <iosfwd>
#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /** Metadata information about a file
   *
   *  See BELLE2-NOTE-TE-2015-028: Event, File, and Dataset Metadata for a
   *  detailed definition. Available at: https://docs.belle2.org/record/287?ln=en
   */
  class DQMFileMetaData : public TObject {
  public:
    /** Constructor.
     */
    DQMFileMetaData();

    /** Number of events getter.
     */
    unsigned int getNEvents() const {return m_nEvents;}

    /** Lowest experiment number getter.
     */
    int getExperimentLow() const {return m_experimentLow;}

    /** Lowest run number getter.
     */
    int getRunLow() const {return m_runLow;}

    /** Lowest event number in lowest run getter.
     */
    unsigned int getEventLow() const {return m_eventLow;}

    /** Highest experiment number getter.
     */
    int getExperimentHigh() const {return m_experimentHigh;}

    /** Highest run number getter.
     */
    int getRunHigh() const {return m_runHigh;}

    /** Highest event number in highest run getter.
     */
    unsigned int getEventHigh() const {return m_eventHigh;}

    /** File creation date and time getter (UTC) */
    const std::string& getDate() const {return m_date;}

    /** Software release version getter.
     */
    const std::string& getRelease() const {return m_release;}

    /** Is it generated data?.
     */
    bool isMC() const {return m_isMC;}

    /** Number of generated events getter.
     */
    unsigned int getMcEvents() const {return m_mcEvents;}

    /** Get the database global tag used when creating this file. If more then
     * one global tag was used by multiple conditions database instances all
     * are concatenated using ',' as separation. If no conditions database was
     * used an empty string is returned
     */
    const std::string& getDatabaseGlobalTag() const { return m_databaseGlobalTag; }

    /** Number of events setter.
     *
     *  @param nEvents The number of events.
     */
    void setNEvents(unsigned int nEvents) {m_nEvents = nEvents;}

    /** Lowest experiment, run and event number setter.
     *
     *  @param experiment The lowest experiment number.
     *  @param run The lowest run number.
     *  @param event The lowest event number of the lowest run.
     */
    void setLow(int experiment, int run, unsigned int event) {m_experimentLow = experiment; m_runLow = run; m_eventLow = event;}

    /** Highest experiment, run and event number setter.
     *
     *  @param experiment The highest experiment number.
     *  @param run The highest run number.
     *  @param event The highest event number of the highest run.
     */
    void setHigh(int experiment, int run, unsigned int event) {m_experimentHigh = experiment; m_runHigh = run; m_eventHigh = event;}

    /** Creation data setter.
     *
     *  @param date The creation date and time.
     *  @param site The site where the file was created.
     *  @param user The user who created the file.
     *  @param release The software release.
     */
    void setCreationData(const std::string& date, const std::string& release)
    {m_date = date; m_release = release;}


    /** Number of generated events setter.
     *
     *  @param nEvents The number of generated events.
     */
    void setMcEvents(unsigned int nEvents) {m_mcEvents = nEvents;}

    /** Set the database global tag used when creating this file. If more then
     * one global tag was used by multiple conditions database instances all
     * should be concatenated using ',' as separation. If no conditions
     * database was used an empty string should be set.
     */
    void setDatabaseGlobalTag(const std::string& globalTag) { m_databaseGlobalTag = globalTag; }

    /** Print the content of the meta data object.
     *
     *  @param option Use "all" to print everything, except steering file. Use "steering" for printing (only) steering file. "json" for machine-readable output
     */
    virtual void Print(Option_t* option = "") const override;

    /** Get a json representation.
     */
    std::string getJsonStr() const;

    bool write(std::ostream& output, const std::string& physicalFileName) const;

  private:

    unsigned int m_nEvents; /**< Number of events.  */

    int m_experimentLow; /**< Lowest experiment number.  */

    int m_runLow; /**< Lowest run number.  */

    unsigned int m_eventLow; /**< Lowest event number in lowest run.  */

    int m_experimentHigh; /**< Highest experiment number.  */

    int m_runHigh; /**< Highest run number.  */

    unsigned int m_eventHigh; /**< Highest event number in highest run.  */

    std::string m_date; /**< File creation date and time (UTC).  */

    std::string m_release; /**< Software release version.  */

    bool m_isMC; /**< Is it generated or real data?.  */

    unsigned int m_mcEvents; /**< Number of generated events, 0 for real data.  */

    std::string m_databaseGlobalTag; /**< Global tag in the database used for production of this file */

    /** Declare that this is not generated, but real data.
     */
    void declareMCData() {m_isMC = true;}

    ClassDefOverride(DQMFileMetaData, 1); /**< Metadata information about a file. */

  }; //class

} // namespace Belle2
