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
#include <TTimeStamp.h>
#include <map>
#include <tuple>

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
    ARICHDatabaseImporter(): m_inputFilesHapdQA(), m_inputFilesAsicRoot(), m_inputFilesAsicTxt(), m_inputFilesHapdQE() {};

    /**
     * Constructor
     */
    ARICHDatabaseImporter(std::vector<std::string> inputFilesHapdQA, std::vector<std::string> inputFilesAsicRoot,
                          std::vector<std::string> inputFilesAsicTxt, std::vector<std::string> inputFilesHapdQE);

    /**
     * Destructor
     */
    virtual ~ARICHDatabaseImporter() {};

    /**
     * Import ARICH aerogel data in the database.
     */
    void importAerogelInfo();

    /**
     * Export ARICH aerogel data from the database.
     */
    void exportAerogelInfo();

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
    TTimeStamp getAsicDate(std::string asicSerial, std::string type);

    /**
     * Get lists of problematic ASIC channels.
     */
    std::vector<int> channelsList(std::string asicSerial, std::string type, int chDelay);

    /**
     * Import ARICH FEB test data in the database.
     */
    void importFebTest();

    /**
     * Returns data from low voltage test
     */
    std::tuple<std::string, float, float, float> getFebLVtestData(int serial);

    /**
     * Returns data from high voltage test
     */
    std::tuple<std::string, float> getFebHVtestData(int serial);

    /**
     * Returns list of dead channels on FEB
     */
    std::vector<int> getDeadChFEB(std::string dna);

    /**
     * Convert date (FEB) to TTimeStamp.
     * @param time - time as string
     */
    TTimeStamp timedate2(std::string time);

    /**
     * Returns lists of slopes (fine & rough)
     */
    std::pair<std::vector<float>, std::vector<float>> getSlopes(std::string dna);

    /**
     * Export ARICH FEB test data from the database.
     */
    void exportFebTest();

    /**
     * Import ARICH HAPD data and chip data in the database.
     */
    void importHapdInfo();

    /**
     * Get lists of problematic HAPD channels.
     */
    std::vector<int> channelsListHapd(std::string chlist, int channelDelay);

    /**
     * Get position of channel on HAPD.
     * @param XY - choose X/Y coordinate
     * @param chip_2d - chip label
     * @param chipnum - channel number
     */
    int getChannelPosition(std::string XY, std::string chip_2d, int chipnum);

    /**
     * Get graphs for bombardment and avalanche gain and current.
     * @param bomb_val - bombardment or avalanche
     * @param g_i - gain or current
     * @param chip_label - chip label
     * @param i - number of entries
     * @param HV - voltage
     * @param gain_current - gain/current
     */
    TGraph* getGraphGainCurrent(std::string bomb_aval, std::string g_i, std::string chip_label, int i, float* HV, float* gain_current);

    /**
     * Get histograms for bias voltage and current.
     * @param chip_2d - chip label
     * @param voltage_current - voltage or current
     * @param chipnum - channel number
     * @param bias_v_i - bias voltage/current
     */
    TH2F* getBiasGraph(std::string chip_2d, std::string voltage_current, int* chipnum, float* bias_v_i);

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
     * Import lists of bad channels in the database.
     */
    void importBadChannels();

    /**
     * Export lists of bad channels from the database.
     */
    void exportBadChannels();

    /**
     * Export ARICH HAPD chip info data from the database and calculate bias voltages for one HAPD.
     * @param HAPD serial number
     */
    void getBiasVoltagesForHapdChip(std::string serialNumber);

    /**
     * Example that shows how to use data from the database
     * @param aerogel serial number
     */
    void getMyParams(std::string aeroSerialNumber);

    /**
     * Function that returns refractive index, thickness and transmission length of aerogel
     * @param aerogel serial number
     */
    std::map<std::string, float> getAerogelParams(std::string aeroSerialNumber);

    /**
     * Import module sensor map and info classes
     */
    void importSensorModule();

    /**
     * Export module sensor map and info classes from database
     */
    void exportSensorModule();

  private:

    std::vector<std::string> m_inputFilesHapdQA;        /**< Input root files for HAPD QA */
    std::vector<std::string> m_inputFilesAsicRoot;      /**< Input root files for ASICs */
    std::vector<std::string> m_inputFilesAsicTxt;       /**< Input txt files for ASICs */
    std::vector<std::string> m_inputFilesHapdQE;        /**< Input root files for HAPD quantum efficiency */

    ClassDef(ARICHDatabaseImporter, 1);                 /**< ClassDef */
  };

} // Belle2 namespace
#endif
