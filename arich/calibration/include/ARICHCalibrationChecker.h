/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * ARICH calibration checker.
   */
  class ARICHCalibrationChecker {

  public:

    /**
     * Constructor.
     */
    ARICHCalibrationChecker();

    /**
     * Destructor.
     */
    ~ARICHCalibrationChecker();

    /**
     * Set experiment and run numbers.
     */
    void setExperimentRun(int experiment, int run);

    /**
     * Set testing payload name.
     */
    void setTestingPayload(const std::string& testingPayloadName)
    {
      m_testingPayloadName = testingPayloadName;
    }

    /**
     * Set Global Tag name.
     */
    void setGlobalTag(const std::string& globalTagName)
    {
      m_GlobalTagName = globalTagName;
    }

    /**
     * Set channel mask results file.
     */
    void setChannelMaskResultsFile(
      const std::string& channelMaskResultsFile)
    {
      m_channelMaskResultsFile = channelMaskResultsFile;
    }

    /**
     * Check channel mask.
     */
    void checkChannelMask();

    /**
     * Get HAPD sector number
     */
    int getSector(int modID);

    /**
     * Get HAPD ring number
     */
    int getRing(int modID);

  private:

    /**
     * Initialize the database.
     */
    void initializeDatabase();

    /**
     * Reset the database.
     */
    void resetDatabase();

    /**
     * Print payload information.
     */
    template<class T> void printPayloadInformation(DBObjPtr<T>& dbObject)
    {
      B2INFO("Analyzing the following payload:"
             << LogVar("Global Tag", m_GlobalTagName.c_str())
             << LogVar("Name", dbObject.getName())
             << LogVar("Revision", dbObject.getRevision())
             << LogVar("IoV", dbObject.getIoV()));
    }

    /** Experiment number. */
    int m_experiment;

    /** Run number. */
    int m_run;

    /** Testing payload location. */
    std::string m_testingPayloadName = "";

    /** Global Tag name. */
    std::string m_GlobalTagName = "";

    /** Output file for channel mask results. */
    std::string m_channelMaskResultsFile = "channel_mask.root";

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

  };

}

