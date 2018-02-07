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
     * Import channel-by-channel T0 calibration constants to database
     * The input is the root file produced by TOPLaserCalibrator
     * @param fileNames file names separated by space (since vector doesn't work!)
     */
    void importLocalT0Calibration(std::string fileNames);


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
     * Import PMT Quantum Efficiency data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtQEData(std::string fileName, std::string treeName);

    /**
     * Import PMT gain parameters data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtGainData(std::string fileName, std::string treeName);

    /**
     * Import PMT installation data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtInstallationData(std::string fileName, std::string treeName);

    /**
     * Import PMT specifications from Hamamatsu (not to be used!)
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtObsoleteData(std::string fileName, std::string treeName);

    /**
     * Import gaussians fitting the TTS distributions
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtTTSPar(std::string fileName, std::string treeName);

    /**
     * Import histograms used for PMT TTS determination
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtTTSHisto(std::string fileName, std::string treeName);

    /**
     * Example of exporting TTS histograms
     * @param outFilefileName : name of the root file where data will be saved
     */
    void exportPmtTTSHisto(std::string outFileName);

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
