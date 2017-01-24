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
#include <vector>

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
     * Import sample time calibration constants to database
     * Kichimi-san data, one text file per module - for format see function code
     * @param fileNames file names separated by space (since vector doesn't work!)
     */
    void importSampleTimeCalibrationKichimi(std::string fileNames);

    /**
     * Print sample time calibration constants stored in database
     */
    void printSampleTimeCalibration();

    /**
     * for testing purposes only! - will be removed ...
     */
    void importTest(int runNumber, double syncTimeBase);
    void importTest();

  private:

    // ClassDef(TOPDatabaseImporter, 1);  /**< ClassDef - why this is needed? */
  };

} // Belle2 namespace
