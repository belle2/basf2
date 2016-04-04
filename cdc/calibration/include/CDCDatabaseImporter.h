/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cdc/geometry/CDCGeometryPar.h>
#include <string>

namespace Belle2 {

  /**
   * CDC database importer.
   */
  class CDCDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    CDCDatabaseImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * Constructor
     */
    CDCDatabaseImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }


    /**
     * Destructor
     */
    virtual ~CDCDatabaseImporter()
    {}

    /**
     * Import channel map to the data base.
     */
    void importChannelMap(std::string fileName);
    /**
     * Get the channel map from the database and
     * print it.
     */
    void printChannelMap();

    /**
     * Import t0 table to the data base.
     */
    void importTimeZero();
    /**
     * Get the t0 table from the database and
     * print it.
     */
    void printTimeZero();

  private:

    /**
     * CDC geometory parameter.
     */
    CDC::CDCGeometryPar& m_cdcgp = CDC::CDCGeometryPar::Instance();
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */
    ClassDef(CDCDatabaseImporter, 1);  /**< ClassDef */
  };

} // Belle2 namespace
