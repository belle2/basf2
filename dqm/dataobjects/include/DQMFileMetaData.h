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

    /** experiment number getter.
     */
    int getExperiment() const {return m_experiment;}

    /** run number getter.
     */
    int getRun() const {return m_run;}

    /** Run date getter (UTC) */
    const std::string& getRunDate() const {return m_date;}

    /** Software release version getter */
    const std::string& getRelease() const {return m_release;}

    /** Processing ID */
    const std::string& getProcessingID() const {return m_procID;}

    /** Run type */
    const std::string& getRunType() const {return m_rtype;}

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

    /** Set experiment and run.
     *
     *  @param experiment experiment number.
     *  @param run run number.
     */
    void setExperimentRun(int experiment, int run) {m_experiment = experiment; m_run = run;}

    void setRelease(const std::string& release)
    {
      m_release = release;
    }

    void setRunDate(const std::string& date)
    {
      m_date = date;
    }

    void setProcessingID(const std::string& procID)
    {
      m_procID = procID;
    }

    void setRunType(const std::string& rtype)
    {
      m_rtype = rtype;
    }

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

    int m_experiment; /**< experiment number.  */

    int m_run; /**< run number.  */

    std::string m_date; /**< run date and time (UTC).  */

    std::string m_release; /**< Software release version.  */

    std::string m_procID; /**< processing ID */

    std::string m_rtype; /**< run type (physics, cosmics, etc.) */

    bool m_isMC; /**< Is it generated or real data?.  */

    unsigned int m_mcEvents; /**< Number of generated events, 0 for real data.  */

    std::string m_databaseGlobalTag; /**< Global tag in the database used for production of this file */

    /** Declare that this is not generated, but real data.
     */
    void declareMCData() {m_isMC = true;}

    ClassDefOverride(DQMFileMetaData, 1); /**< Metadata information about a file. */

  }; //class

} // namespace Belle2
