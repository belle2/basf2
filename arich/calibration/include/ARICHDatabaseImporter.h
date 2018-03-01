/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ARICHDATABASEIMPORTER_H
#define ARICHDATABASEIMPORTER_H

#include <TObject.h>
#include <TGraph.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TTimeStamp.h>
#include <map>
#include <tuple>
#include <vector>
#include <iostream>


namespace Belle2 {

  //! ARICH database importer.
  /*!
    This module writes data to database.
  */
  class ARICHDatabaseImporter {

  public:
    /**
     * Default constructor
     */
    ARICHDatabaseImporter(): m_inputFilesHapdQA(), m_inputFilesAsicRoot(), m_inputFilesAsicTxt(), m_inputFilesHapdQE(),
      m_inputFilesFebTest() {};

    /**
     * Constructor
     */
    ARICHDatabaseImporter(const std::vector<std::string>& inputFilesHapdQA, const std::vector<std::string>& inputFilesAsicRoot,
                          const std::vector<std::string>& inputFilesAsicTxt, const std::vector<std::string>& inputFilesHapdQE,
                          const std::vector<std::string>& inputFilesFebTest);

    /**
     * Destructor
     */
    virtual ~ARICHDatabaseImporter() {};


    // classes used in simulation/reconstruction software

    /**
     * Import simulation parameters from the xml file (QE curve, etc.)
     */
    void importSimulationParams();

    /**
     * Import reconstruction parameters (for now only initializes "default" values and imports)
     */
    void importReconstructionParams();


    /**
     * Print simulation parameters from the database (QE curve, etc.)
     */
    void printSimulationPar();

    /**
     * Import HAPD modules info from the xml file and database (2D QE maps)
     * Goes through the list of installed modules in ARICH-InstalledModules.xml,
     * finds corresponding 2D QE maps in the database and imports lightweight
     * ARICHModulesInfo class (which is used in sim/rec) into database
     */
    void importModulesInfo();

    /**
     * Print HAPD modules info from the database (lightweight class for sim/rec=)
     */
    void printModulesInfo();

    /**
     *  Example function for importing HAPD QE to database class (ARICHModulesInfo)
     */
    void setHAPDQE(unsigned modID, double qe = 0.27, bool import = false);

    /**
     * Import channel mask for all HAPD modules from the database (list of dead channels)
     * Goes through the list of installed modules in ARICH-InstalledModules.xml,
     * finds corresponding lists of dead channels in the database and imports lightweight
     * ARICHChannelMask class (which is used in sim/rec) into database
     */
    void importChannelMask();

    /**
     * Print channel mask of all HAPD modules from the database (lightweight class for sim/rec)
     */
    void printChannelMask();

    /**
     * Imports HAPD (asic) channel mappings from the xml file
     */
    void importChannelMapping();

    /**
     * Prints HAPD (asic) channel mapping from the database
     */
    void printChannelMapping();

    /**
     * Imports mappings of FE electronics from the xml file (ARICH-FrontEndMapping.xml) to the database
     * Mapping of modules to mergers and mergers to coppers
     */
    void importFEMappings();

    /**
     * Prints mappings of FE electronics from the database
     */
    void printFEMappings();

    /**
     * Prints geometry configuration parameters from the database
     */
    void printGeometryConfig();

    /**
     * Dumps 2D QE map of full detector surface from the database into root file (from ARICHModulesInfo)
     * @param simple false for proper histogram with bin for each channel,
     *               true for simpler plot with point for each channel (smaller file, faster)
     * for simple option, draw TGraph2D with "pcolz" option and set view to "top"
     */
    void dumpQEMap(bool simple = false);

    /**
     * Dumps module numbering scheme into root file (module position on the detector plane -> module number)
     */
    void dumpModuleNumbering();

    /**
     * Import parameters of the cosmic test geometry configuration
     */
    void importCosmicTestGeometry();

    /**
     * Import geometry configuration parameters to the database
     */
    void importGeometryConfig();

    /**
    * Import optical information of aerogel tiles into database
    */
    void importAeroTilesInfo();

    /**
     * Get aerogel ring number from global indetifier
     */
    int getAeroTileRing(int slot);

    /**
     * Get aerogel ring number from global indetifier
     */
    int getAeroTileColumn(int slot);

    /**
     * Prints mapping of aerogel tiles and their optical properties
     */
    void printAeroTileInfo();


    // DAQ classes

    /**
     * Imports mappings of power supply to bias cables and cables to HAPDs and nominal values of bias voltages
     */
    void importBiasMappings();

    /**
     * Imports mappings of power supply to high voltage cables
     */
    void importHvMappings();

    /**
     * Imports mappings of nominal values of bias voltages
     */
    void importNominalBiasVoltages();

    /**
     * Prints mappings of power supply to bias cables and cables to HAPDs and nominal values of bias voltages from the database
     */
    void printBiasMappings();

    /**
     * Prints mappings of power supply to HV cables and cables to HAPDs from the database
     */
    void printHvMappings();

    /**
     * Prints mappings of nominal values of bias voltages from the database
     */
    void printNominalBiasVoltages();

    /**
     * Prints nominal bias voltage for channel on power supply from the database
     * @param channel vector of values crate, slot and channel on power supply
     */
    void printNominalBiasVoltageForChannel(std::vector<int> channel);

    /**
     * Prints HAPD position (sector, ring, azimuth) and merger connection (merger, feb slot) from the database
     * @param crate crate on power supply
     * @param slot slot on power supply
     * @param channelID channel number on power supply
     */
    void printHapdPositionFromCrateSlot(int crate, int slot, int channelID);

    /**
     * Returns feb daq position from feb slot
     * @param febSlot feb slot number on merger
     * @return feb daq number
     */
    int getFebDaqSlot(unsigned febSlot);


    // classes used in conditions DB

    /**
     * Import ARICH aerogel data in the database.
     */
    void importAerogelInfo();

    /**
     * Export ARICH aerogel data from the database.
     */
    void exportAerogelInfo(int verboseLevel = 0);

    /**
     * Import ARICH aerogel map in the database.
     */
    void importAerogelMap();

    /**
     * Export ARICH aerogel map in the database.
     */
    void exportAerogelMap();

    /**
     * Import intrarun dependant ARICH aerogel data in the database. -> Example for intrarun dependat data!
     */
    void importAerogelInfoEventDep();

    /**
     * Export intrarun dependant ARICH aerogel data from the database. -> Example for intrarun dependat data!
     */
    void exportAerogelInfoEventDep();

    /**
     * Import ARICH HAPD QA data in the database.
     */
    void importHapdQA();

    /**
     * Export ARICH HAPD QA data from the database.
     */
    void exportHapdQA();

    /**
     * Import ARICH ASICs data in the database.
     */
    void importAsicInfo();

    /**
     * Import large histograms from ARICH ASICs data in the database.
     */
    void importAsicInfoRoot();

    /**
     * Export ARICH ASICs data from the database.
     */
    void exportAsicInfo();

    /**
     * Convert date (ASICs) to TTimeStamp.
     * @param enddate - time as string
     */
    TTimeStamp timedate(std::string enddate);

    /**
     * Get date for ASIC measurement.
     * @param asicSerial - serial number of asic
     * @param type - gain or offset
     */
    TTimeStamp getAsicDate(const std::string& asicSerial, const std::string& type);

    /**
     * Get lists of problematic ASIC channels.
     */
    std::vector<int> channelsList(std::string badCH);

    /**
     * Import ARICH FEB test data in the database.
     */
    void importFebTest();

    /**
     * Import large histograms from ARICH FEB test data in the database.
     */
    void importFebTestRoot();

    /**
     * Returns data from low voltage test
     */
    std::tuple<std::string, float, float, float> getFebLVtestData(int serial, int lvRun);

    /**
     * Returns data from high voltage test
     */
    std::tuple<std::string, float> getFebHVtestData(int serial, int hvRun);

    /**
     * Returns list of dead channels on FEB
     */
    std::vector<int> getDeadChFEB(const std::string& dna);

    /**
     * Convert date (FEB) to TTimeStamp.
     * @param time - time as string
     */
    TTimeStamp timedate2(std::string time);

    /**
     * Returns lists of slopes (fine & rough)
     */
    std::pair<std::vector<float>, std::vector<float>> getSlopes(int serialNum, const std::string& runSCAN);

    /**
     * Returns lists of FWHM values&sigmas
     */
    std::vector<std::pair<float, float>> getFwhm(int serialNum, const std::string& runSCAN);

    /**
     * Returns TH3F histograms - offset settings
     */
    std::vector<TH3F*> getFebTestHistograms(const std::string& dna, const std::string& run, int febposition);

    /**
     * Returns TH2F histogram of pulse test
     */
    TH2F* getFebTestPulse(const std::string& dna, const std::string& run, int febposition);

    /**
     * Export ARICH FEB test data from the database.
     */
    void exportFebTest();

    /**
     * Import ARICH HAPD chip data in the database.
     */
    void importHapdChipInfo();

    /**
     * Export ARICH HAPD chip data from the database.
     */
    void exportHapdChipInfo();

    /**
     * Import ARICH HAPD data in the database.
     */
    void importHapdInfo();

    /**
     * Get lists of problematic HAPD channels.
     */
    std::vector<int> channelsListHapd(std::string chlist, std::string chipDelay);

    /**
     * Get position of channel on HAPD.
     * @param XY - choose X/Y coordinate
     * @param chip_2d - chip label
     * @param chipnum - channel number
     */
    int getChannelPosition(const std::string& XY, const std::string& chip_2d, int chipnum);

    /**
     * Get graphs for bombardment and avalanche gain and current.
     * @param bomb_val - bombardment or avalanche
     * @param g_i - gain or current
     * @param chip_label - chip label
     * @param i - number of entries
     * @param HV - voltage
     * @param gain_current - gain/current
     */
    TGraph* getGraphGainCurrent(const std::string& bomb_aval, const std::string& g_i, const std::string& chip_label, int i, float* HV,
                                float* gain_current);

    /**
     * Get histograms for bias voltage and current.
     * @param chip_2d - chip label
     * @param voltage_current - voltage or current
     * @param chipnum - channel number
     * @param bias_v_i - bias voltage/current
     */
    TH2F* getBiasGraph(const std::string& chip_2d, const std::string& voltage_current, int* chipnum, float* bias_v_i);

    /**
     * Export ARICH HAPD info and chip info data from the database.
     */
    void exportHapdInfo();

    /**
     * Import HAPD quantum efficiency in the database.
     */
    void importHapdQE();

    /**
     * Export HAPD quantum efficiency from the database.
     */
    void exportHapdQE();

    /**
     * Export ARICH HAPD chip info data from the database and calculate bias voltages for one HAPD.
     * @param HAPD serial number
     */
    void printBiasVoltagesForHapdChip(const std::string& serialNumber);

    /**
     * Example that shows how to use data from the database
     * @param aerogel serial number
     */
    void printMyParams(const std::string& aeroSerialNumber);

    /**
     * Function that returns refractive index, thickness and transmission length of aerogel
     * @param aerogel serial number
     */
    std::map<std::string, float> getAerogelParams(const std::string& aeroSerialNumber);

    /**
     * Import module test results
     */
    void importFEBoardInfo();

    /**
     * Export module test results
     */
    void exportFEBoardInfo();

    /**
     * Import module test results
     */
    void importModuleTest(const std::string& mypath, const std::string& HVtest);

    /**
     * Export module test results
     */
    void exportModuleTest(const std::string& HVtest);

    /**
     * Import module sensor info classes
     */
    void importSensorModuleInfo();

    /**
     * Import module sensor map classes
     */
    void importSensorModuleMap();

    /**
     * Export module sensor map and info classes from database
     */
    void exportSensorModuleMap();
    void exportSensorModuleMapInfo(int number);

    /**
     * Import results of magnet test
     */
    void importMagnetTest();

    /**
     * Export results of magnet test
     */
    void exportMagnetTest();

    /**
     * Export all the data
     */
    void exportAll();


  private:

    std::vector<std::string> m_inputFilesHapdQA;        /**< Input root files for HAPD QA */
    std::vector<std::string> m_inputFilesAsicRoot;      /**< Input root files for ASICs */
    std::vector<std::string> m_inputFilesAsicTxt;       /**< Input txt files for ASICs */
    std::vector<std::string> m_inputFilesHapdQE;        /**< Input root files for HAPD quantum efficiency */
    std::vector<std::string> m_inputFilesFebTest;       /**< Input root files from FEB test (coarse/fine offset settings, test pulse) */


    /**
     * @brief printContainer used for debugging purposes...
     * @param rContainer
     * @param rStream
     */
    template <typename Container_t>
    inline auto printContainer(const Container_t& rContainer, std::ostream& rStream = std::cout) noexcept -> void
    {
      if (rContainer.empty())
        return void();

      rStream << rContainer.front();
      for (auto i = 1ul; i < rContainer.size(); ++i)
        rStream << " - " << rContainer[i];
      rStream << '\n';
    }

    ClassDef(ARICHDatabaseImporter, 4);                 /**< ClassDef */

  };

} // Belle2 namespace
#endif
