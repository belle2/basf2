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
    void importTimeZero(std::string fileName);
    /**
     * Get the t0 table from the database and
     * print it.
     */
    void printTimeZero();

    /**
     * Import ADC delta pedestal table to the data base specifying the text data.
     */
    void importADCDeltaPedestal(std::string fileName);

    /**
     * Import ADC delta pedestal with all 0.
     */
    void importADCDeltaPedestal();

    /**
     * Get the ADC delta pedestal table from the database and
     * print it.
     */
    void printADCDeltaPedestal();

    /**
     * Import badwire table to the data base.
     */
    void importBadWire(std::string fileName);
    /**
     * Get the badwire table from the database and
     * print it.
     */
    void printBadWire();

    /**
     * Import propspeed table to the database.
     */
    void importPropSpeed(std::string fileName);
    /**
     * Get the propspeed table from the database and
     * print it.
     */
    void printPropSpeed();

    /**
     * Import time-walk coeff. table to the database.
     */
    void importTimeWalk(std::string fileName);
    /**
     * Get the time-walk coeff. table from the database and
     * print it.
     */
    void printTimeWalk();

    /**
     * Import xt table to the database.
     */
    void importXT(std::string fileName);
    /**
     * Get the xt table from the database and
     * print it.
     */
    void printXT();

    /**
     * Import sigma table to the database.
     */
    void importSigma(std::string fileName);
    /**
     * Get the sigma table from the database and
     * print it.
     */
    void printSigma();

    /**
     * Import displacement of wire position to the database.
     */
    void importDisplacement(std::string fileName);

    /**
     * Get the displacement of wire position from the database and
     * print it.
     */
    void printDisplacement();

    /**
     * Import wire alignment table to the database.
     */
    void importWirPosAlign(std::string fileName);
    /**
     * Get the wire alignment table from the database and
     * print it.
     */
    void printWirPosAlign();

    /**
     * Import wire misalignment table to the database.
     */
    void importWirPosMisalign(std::string fileName);
    /**
     * Get the wire misalignment table from the database and
     * print it.
     */
    void printWirPosMisalign();

  private:

    /**
     * CDC geometory parameter.
     */
    //    CDC::CDCGeometryPar& m_cdcgp = CDC::CDCGeometryPar::Instance();
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */
    ClassDef(CDCDatabaseImporter, 1);  /**< ClassDef */
  };

} // Belle2 namespace
