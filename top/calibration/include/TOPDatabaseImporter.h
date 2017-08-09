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
     * Output of TOPTimeBaseCalibrator (root files with histograms)
     * @param fileNames file names separated by space (since vector doesn't work!)
     */
    void importSampleTimeCalibration(std::string fileNames);

    /**
     * Import sample time calibration constants to database
     * Kichimi-san data, root files
     * @param fileNames file names separated by space (since vector doesn't work!)
     */
    void importSampleTimeCalibrationKichimi(std::string fileNames);

    /**
     * Import channel T0 calibration constants to database
     * Kichimi-san data, root files
     * @param fileNames file names separated by space (since vector doesn't work!)
     */
    void importChannelT0CalibrationKichimi(std::string fileNames);

    /**
     * Prints sample time calibration info about constants stored in database
     */
    void getSampleTimeCalibrationInfo();

    /**
     * Print sample time calibration constants stored in database
     */
    void printSampleTimeCalibration();

    /**
     * Generate and import a (random, fake) channel mask for testing
     * @param fractionDead the fraction of dead PMs to randomly assign
     * @param fractionHot the fraction of noisy PMs to randomly assign
     */
    void generateFakeChannelMask(double fractionDead, double fractionHot);

    /**
     * Generate and import a channel mask of all but one module (for probability
     * distrubtion debugging)
     * @param moduleToKeep the module not to be removed (all others are masked)
     */
    void maskAllButOneModule(int moduleToKeep);

    /**
     * for testing purposes only! - will be removed ...
     */
    void importTest(int runNumber, double syncTimeBase);

    /**
     * for testing purposes only! - will be removed ...
     */
    void importTest();

  private:

    // ClassDef(TOPDatabaseImporter, 1);  /**< ClassDef - why this is needed? */
  };

} // Belle2 namespace
