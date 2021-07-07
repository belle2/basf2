/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <iosfwd>
#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /**
   * Metadata information about a DQM file
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

    /** Processing ID getter */
    const std::string& getProcessingID() const {return m_procID;}

    /** Run type getter */
    const std::string& getRunType() const {return m_rtype;}

    /** Is it generated data?.
     */
    bool isMC() const {return m_isMC;}

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

    /** set release
     *  @param release release number
     */
    void setRelease(const std::string& release)
    {
      m_release = release;
    }

    /** set run date
     *  @param date date and time of run
     */
    void setRunDate(const std::string& date)
    {
      m_date = date;
    }

    /** set processing ID
     *  @param procID processing ID (online,proc9,proc10,etc.)
     */
    void setProcessingID(const std::string& procID)
    {
      m_procID = procID;
    }

    /** set run type
     *  @param rtype run type (physics,cosmic,local,null)
     */
    void setRunType(const std::string& rtype)
    {
      m_rtype = rtype;
    }

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

  private:

    unsigned int m_nEvents; /**< Number of events.  */

    int m_experiment; /**< experiment number.  */

    int m_run; /**< run number.  */

    std::string m_date; /**< run date and time (UTC).  */

    std::string m_release; /**< Software release version.  */

    std::string m_procID; /**< processing ID */

    std::string m_rtype; /**< run type (physics, cosmics, etc.) */

    bool m_isMC; /**< Is it generated or real data?.  */

    std::string m_databaseGlobalTag; /**< Global tag in the database used for production of this file */

    /** Declare that this is not generated, but real data.
     */
    void declareMCData() {m_isMC = true;}

    ClassDefOverride(DQMFileMetaData, 1); /**< Metadata information about a file. */

  }; //class

} // namespace Belle2
