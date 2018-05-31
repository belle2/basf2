/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Umberto Tamponi                            *
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
    void importSampleTimeCalibration(std::string fileNames, int firstExp, int lastExp, int firstRun, int lastRun);


    /**
     * Import channel-by-channel T0 calibration constants to database
     * The input is the root file produced by TOPLaserCalibrator
     * @param fileNames file names separated by space (since vector doesn't work!)
     */
    void importLocalT0Calibration(std::string fileNames, int firstExp, int lastExp, int firstRun, int lastRun);


    /**
     * Import module T0 calibration constants to database
     * The input is the text file
     * @param fileName name of the dat file with constants of all modules
     */
    void importModuleT0Calibration(std::string fileName, int firstExp, int lastExp, int firstRun, int lastRun);


    /**
     * Import common T0 calibration constants derived form the offline data reprocessing to database
     * The input is a root file containing a tree (one per run). In the future we may add a furter implementation
     * that reads the IOV from the root file itself.
     * @param fileName name of the root file with constants of all modules
     */
    void importOfflineCommonT0Calibration(std::string fileName, int firstExp, int lastExp, int firstRun, int lastRun);


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
    void generateFakeChannelMask(double fractionDead, double fractionHot, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import PMT Quantum Efficiency data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtQEData(std::string fileName, std::string treeName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import PMT gain parameters data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtGainData(std::string fileName, std::string treeName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import PMT installation data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtInstallationData(std::string fileName, std::string treeName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import PMT specifications from Hamamatsu (not to be used!)
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtObsoleteData(std::string fileName, std::string treeName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import gaussians fitting the TTS distributions
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtTTSPar(std::string fileName, std::string treeName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import histograms used for PMT TTS determination
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     */
    void importPmtTTSHisto(std::string fileName, std::string treeName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Import fit results of pulse height disribution for channel gain and threshold efficiency
     * @param fileName : name of the root file containing relevant data, which is obtained from TOPGainEfficiencyMonitor
     */
    void importPmtPulseHeightFitResult(std::string fileName, int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * Example of exporting TTS histograms
     * @param outFilefileName : name of the root file where data will be saved
     */
    void exportPmtTTSHisto(std::string outFileName);


    /**
     * import a dummy payload of TOPCalModuleAlignment DB objects
     */
    void importDummyCalModuleAlignment(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalCalModuleT0 DB objects
     */
    void importDummyCalModuleT0(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalChannelT0 DB objects
     */
    void importDummyCalChannelT0(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalTimebase DB objects
     */
    void importDummyCalTimebase(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalChannelNoise DB objects
     */
    void importDummyCalChannelNoise(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalChannelPulseHeight DB objects
     */
    void importDummyCalChannelPulseHeight(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalChannelRQE DB objects
     */
    void importDummyCalChannelRQE(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalChannelThresholdEff DB objects
     */
    void importDummyCalChannelThresholdEff(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalChannelThreshold DB objects
     */
    void importDummyCalChannelThreshold(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalCommonT0 DB objects
     */
    void importDummyCalCommonT0(int firstExp, int lastExp, int firstRun, int lastRun);

    /**
     * import a dummy payload of TOPCalIntegratedCharge DB objects
     */
    void importDummyCalIntegratedCharge(int firstExp, int lastExp, int firstRun, int lastRun);


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
