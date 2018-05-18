/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {

  /**
   *  database importer.
   */
  class RunInfoDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    RunInfoDatabaseImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * Constructor
     */
    RunInfoDatabaseImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~RunInfoDatabaseImporter()
    {}

    /**
     * Import the run information to the database.
     */
    void importRunInfo(std::string fileName);

    /**
     * Get the run information from the database and
     * print it.
     */
    void printRunInfo();

  private:

    /**

     */
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */
    //ClassDef(RunInfoDatabaseImporter, 1);  /**< ClassDef */
  };

} // Belle2 namespace
