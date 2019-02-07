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
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importSampleTimeCalibration(std::string fileNames,
                                     int firstExp = 0, int firstRun = 0,
                                     int lastExp = -1, int lastRun = -1);


    /**
     * Import channel-by-channel T0 calibration constants to database
     * The input is the root file with ntuple produced by TOPLaserCalibrator
     * @param fileNames file names separated by space (since vector doesn't work!)
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importLocalT0Calibration(std::string fileNames,
                                  int firstExp = 0, int firstRun = 0,
                                  int lastExp = -1, int lastRun = -1);


    /**
     * Import channel T0 calibration constants
     * The input is a root file with 1D histograms (one per slot, named "channelT0_slot*")
     * @param fileName root file name
     * @param expNo experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastRun last run number of IOV
     */
    void importChannelT0(std::string fileName,
                         int expNo, int firstRun, int lastRun);


    /**
     * Import module T0 calibration constants to database
     * The input is the text file
     * @param fileName name of the dat file with constants of all modules
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importModuleT0Calibration(std::string fileName,
                                   int firstExp = 0, int firstRun = 0,
                                   int lastExp = -1, int lastRun = -1);

    /**
     * Import module T0 calibration constants
     * The input is a root file with 1D histogram (name is "moduleT0")
     * @param fileName root file name
     * @param expNo experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastRun last run number of IOV
     */
    void importModuleT0(std::string fileName,
                        int expNo, int firstRun, int lastRun);

    /**
     * Import common T0 calibration constants derived form the offline data reprocessing to database
     * The input is a root file containing a tree (one per run). In the future we may add a furter implementation
     * that reads the IOV from the root file itself.
     * @param fileName name of the root file with constants of all modules
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importOfflineCommonT0Calibration(std::string fileName,
                                          int firstExp = 0, int firstRun = 0,
                                          int lastExp = -1, int lastRun = -1);


    /**
     * Import common T0 calibration constants
     * @param value central value of T0
     * @param error uncertainty on T0
     * @param expNo experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastRun last run number of IOV
     */
    void importCommonT0(double value, double error,
                        int expNo, int firstRun, int lastRun);

    /**
     * Prints sample time calibration info about constants stored in database
     */
    void getSampleTimeCalibrationInfo();

    /**
     * Print sample time calibration constants stored in database
     */
    void printSampleTimeCalibration();

    /**
     * Import channel mask from a root file (given as 1D histograms, one per slot).
     * @param fileName root file name
     * @param expNo experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastRun last run number of IOV
     */
    void importChannelMask(std::string fileName,
                           int expNo, int firstRun, int lastRun);

    /**
     * Generate and import a (random, fake) channel mask for testing
     * @param fractionDead the fraction of dead PMs to randomly assign
     * @param fractionHot the fraction of noisy PMs to randomly assign
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void generateFakeChannelMask(double fractionDead, double fractionHot,
                                 int firstExp = 0, int firstRun = 0,
                                 int lastExp = -1, int lastRun = -1);

    /**
     * Import PMT Quantum Efficiency data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtQEData(std::string fileName,
                         std::string treeName = "qePmtData",
                         int firstExp = 0, int firstRun = 0,
                         int lastExp = -1, int lastRun = -1);

    /**
     * Import PMT gain parameters data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtGainData(std::string fileName,
                           std::string treeName = "gainPmtData",
                           int firstExp = 0, int firstRun = 0,
                           int lastExp = -1, int lastRun = -1);

    /**
     * Import PMT installation data to database
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtInstallationData(std::string fileName,
                                   std::string treeName = "installationPmtData",
                                   int firstExp = 0, int firstRun = 0,
                                   int lastExp = -1, int lastRun = -1);

    /**
     * Import PMT specifications from Hamamatsu (not to be used!)
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtObsoleteData(std::string fileName,
                               std::string treeName = "obsPmtData",
                               int firstExp = 0, int firstRun = 0,
                               int lastExp = -1, int lastRun = -1);

    /**
     * Import gaussians fitting the TTS distributions
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtTTSPar(std::string fileName,
                         std::string treeName = "ttsPmtPar",
                         int firstExp = 0, int firstRun = 0,
                         int lastExp = -1, int lastRun = -1);

    /**
     * Import histograms used for PMT TTS determination
     * @param fileName : name of the root file containing relevant data
     * @param treeName : name of the tree containing relevant data
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtTTSHisto(std::string fileName,
                           std::string treeName = "ttsPmtHisto",
                           int firstExp = 0, int firstRun = 0,
                           int lastExp = -1, int lastRun = -1);

    /**
     * Import fit results of pulse height disribution for channel gain and threshold efficiency
     * @param fileName : name of the root file containing relevant data, which is obtained from TOPGainEfficiencyMonitor
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importPmtPulseHeightFitResult(std::string fileName,
                                       int firstExp = 0, int firstRun = 0,
                                       int lastExp = -1, int lastRun = -1);

    /**
     * Example of exporting TTS histograms
     * @param outFilefileName : name of the root file where data will be saved
     */
    void exportPmtTTSHisto(std::string outFileName = "RetrievedHistos.root");


    /**
     * import a dummy payload of TOPCalModuleAlignment DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalModuleAlignment(int firstExp = 0, int firstRun = 0,
                                       int lastExp = -1, int lastRun = -1);

    /**
     * import a dummy payload of TOPCalCalModuleT0 DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalModuleT0(int firstExp = 0, int firstRun = 0,
                                int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalChannelT0 DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalChannelT0(int firstExp = 0, int firstRun = 0,
                                 int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalTimebase DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalTimebase(int firstExp = 0, int firstRun = 0,
                                int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalChannelNoise DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalChannelNoise(int firstExp = 0, int firstRun = 0,
                                    int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalChannelPulseHeight DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalChannelPulseHeight(int firstExp = 0, int firstRun = 0,
                                          int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalChannelRQE DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalChannelRQE(int firstExp = 0, int firstRun = 0,
                                  int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalChannelThresholdEff DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalChannelThresholdEff(int firstExp = 0, int firstRun = 0,
                                           int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalChannelThreshold DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalChannelThreshold(int firstExp = 0, int firstRun = 0,
                                        int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalCommonT0 DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalCommonT0(int firstExp = 0, int firstRun = 0,
                                int lastExp = -1, int lastRun = -1);
    /**
     * import a dummy payload of TOPCalIntegratedCharge DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalIntegratedCharge(int firstExp = 0, int firstRun = 0,
                                        int lastExp = -1, int lastRun = -1);

    /**
     * correct QE values in database for the reflection on window surface
     * be sure that you run the function only once!
     * see: BII-4230
     */
    void correctTOPPmtQE();

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
