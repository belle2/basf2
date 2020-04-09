/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * KLM calibration checker.
   */
  class KLMCalibrationChecker {

  public:

    /**
     * Constructor.
     */
    KLMCalibrationChecker();

    /**
     * Destructor.
     */
    ~KLMCalibrationChecker();

    /**
     * Set experiment and run numbers.
     */
    void setExperimentRun(int experiment, int run);

    /**
     * Set testing payload name.
     */
    void setTestingPayload(const std::string& testingPayloadName);

    /**
     * Set Global Tag name.
     */
    void setGlobalTag(const std::string& globalTagName);

    /**
     * Set alignment results file.
     */
    void setAlignmentResultsFile(const std::string& alignmentResultsFile);

    /**
     * Check alignment.
     */
    void checkAlignment();

    /**
     * Check strip efficiency.
     */
    void checkStripEfficiency();

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

    /** Output file for alignment results. */
    std::string m_AlignmentResultsFile = "alignment.root";

  };

}

