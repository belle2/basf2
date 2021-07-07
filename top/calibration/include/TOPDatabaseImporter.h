/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Python.h>
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
     * Import ASIC shifts of BS13d
     * @param s0 shift of carrier 0 [ns]
     * @param s1 shift of carrier 1 [ns]
     * @param s2 shift of carrier 2 [ns]
     * @param s3 shift of carrier 3 [ns]
     * @param expNo experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastRun last run number of IOV
     */
    void importAsicShifts_BS13d(double s0, double s1, double s2, double s3,
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
     * @param roughlyCalibrated if true set payload status to roughly calibrated
     */
    void importCommonT0(double value, double error,
                        int expNo, int firstRun, int lastRun,
                        bool roughlyCalibrated = false);

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
     * @param outFileName name of the root file where data will be saved
     */
    void exportPmtTTSHisto(std::string outFileName = "RetrievedHistos.root");

    /**
     * Import front-end settings
     * @param lookback the number of lookback windows
     * @param readoutWin the number of readout windows
     * @param extraWin the number of extra windows btw. lookback and readout window
     * @param offset offset to photon peak [RF clocks]
     * @param expNo experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastRun last run number of IOV
     */
    void importFrontEndSettings(int lookback, int readoutWin, int extraWin, int offset,
                                int expNo, int firstRun, int lastRun);

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
     * import a dummy payload of TOPCalAsicShift DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importDummyCalAsicShift(int firstExp = 0, int firstRun = 0,
                                 int lastExp = -1, int lastRun = -1);

    /**
     * correct QE values in database for the reflection on window surface
     * be sure that you run the function only once!
     * see: BII-4230
     */
    void correctTOPPmtQE();

    /**
     * payload TOPCalTimeWalk
     * import parameters for time-walk correction and electronic time resolution tuning
     * @param list Python list of parameters of time-walk calibration curve [ns]
     * @param a electronic time resolution: noise term excess parameter [ns]
     * @param b electronic time resolution: quadratic term parameter [ns]
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importTimeWalk(PyObject* list, double a, double b,
                        int firstExp = 0, int firstRun = 0,
                        int lastExp = -1, int lastRun = -1);
    /**
     * for testing purposes only! - will be removed ...
     */
    void importTest(int runNumber, double syncTimeBase);

    /**
     * for testing purposes only! - will be removed ...
     */
    void importTest();


  private:

  };

} // Belle2 namespace
