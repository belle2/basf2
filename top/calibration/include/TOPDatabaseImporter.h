/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>

namespace Belle2 {

  /**
   * TOP database importer.
   * This module writes data to database or retrieves them from database.
   * More functions will come with future...
   */
  class TOPDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    TOPDatabaseImporter()
    {}


    /**
     * Destructor
     */
    virtual ~TOPDatabaseImporter()
    {}

    /**
     * Import sample time calibration constants to database
     * @param fileName text file name with calibration constants
     */
    void importSampleTimeCalibration(std::string fileName);

    /**
     * Print sample time calibration constants stored in database
     */
    void printSampleTimeCalibration();

    /**
     * for testing purposes only! - will be removed ...
     */
    void importTest(int runNumber, double syncTimeBase);

  private:

    // ClassDef(TOPDatabaseImporter, 1);  /**< ClassDef - why this is needed? */
  };

} // Belle2 namespace
