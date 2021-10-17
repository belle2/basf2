/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <TObject.h>

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
    CDCDatabaseImporter(int fexp, int frun, int lexp, int lrun);/*:
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }*/


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
     * Import FEE params. to the data base.
     */
    void importFEElectronics(std::string fileName);
    /**
     * Get FEE params. from the database and print.
     */
    void printFEElectronics();

    /**
     * Import edep-to-adc params. to the data base.
     */
    void importEDepToADC(std::string fileName);
    /**
     * Get edep-to-adc params. from the database and print.
     */
    void printEDepToADC();

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
     * Import fudge factor table to the database.
     */
    void importFFactor(std::string fileName);
    /**
     * Get the fudge factor table from the database and
     * print it.
     */
    void printFFactor();

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

    /**
     * Import CDCWireHits cut values to the database.
     * The cut values are read from a json file.
     * An example of legal json file can be found at
     * cdc/data/CDCWireHitRequirements_example.json
     * In the json file, upper values of -1 stand for unbounded.
     */
    void importCDCWireHitRequirements(const std::string& jsonFileName) const;
    /**
     * Get CDCWireHits cut values from the database and
     * print them.
     */
    void printCDCWireHitRequirements() const;

    /// Import crosstalk library prepared in rootFileName
    void importCDCCrossTalkLibrary(const std::string& rootFileName) const;

    /// Print the content of the crosstalk library
    void printCDCCrossTalkLibrary() const;

    /// Do some basic testing of the CDCCrossTalkLibrary
    void testCDCCrossTalkLibrary(bool spotChecks = false) const;

  private:

    /**
     * CDC geometory parameter.
     */
    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */

    ushort m_firstLayerOffset = 0;      /**< Offset of first layer in case some CDC layers are removed */
    ushort m_superLayerOffset = 0;      /**< Offset of first super layer in case some CDC super layers are removed */
    ushort m_nSenseWires      = 14336;  /**< Number of sense wires in the CDC */

  };

} // Belle2 namespace
