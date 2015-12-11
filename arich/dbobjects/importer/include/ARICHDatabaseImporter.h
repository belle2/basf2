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

namespace Belle2 {

  //! ARICH database importer.
  /*!
    This module writes data to database.
  */
  class ARICHDatabaseImporter {

  public:
    ARICHDatabaseImporter(std::vector<std::string> inputFilesHapdQA, std::vector<std::string> inputFilesAsicRoot,
                          std::vector<std::string> inputFilesAsicTxt, std::vector<std::string> inputFilesHapdQE);

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
     */
    TTimeStamp timedate(std::string enddate);

    /**
     * Get date for ASIC measurement.
     */
    TTimeStamp getAsicDate(std::string asicSerial, std::string type);

    /**
     * Get lists of problematic ASIC channels.
     */
    std::vector<int> channelsList(std::string asicSerial, std::string type);

    /**
     * Import ARICH FEB test data in the database.
     */
    void importFebTest();

    /**
     * Returns list of dead channels on FEB
     */
//    std::vector<int> getDeadChFEB(std::string dna);

    /**
     * Convert date (FEB) to TTimeStamp.
     */
    TTimeStamp timedate2(std::string time);

    /**
     * Export ARICH FEB test data from the database.
     */
    void exportFebTest();

    /**
     * Import ARICH HAPD data in the database.
     */
    void importHapdInfo();

    /**
     * Import ARICH HAPD Chip data in the database.
     */
    void importHapdChipInfo();

    /**
     * Get lists of problematic HAPD channels.
     */
    std::vector<int> channelsListHapd(std::string chlist);

    /**
     * Get position of channel on HAPD.
     */
    int getChannelPosition(std::string XY, std::string chip_2d, int chipnum);

    /**
     * Get graphs for bombardment and avalanche gain and current.
     */
    TGraph* getGraphGainCurrent(std::string bomb_aval, std::string g_i, std::string chip_label, int i, float* HV, float* gain_current);

    /**
     * Get histograms for bias voltage and current.
     */
    TH2F* getBiasGraph(std::string chip_2d, std::string voltage_current, int* chipnum, float* bias_v_i);

    /**
     * Export ARICH HAPD info data from the database.
     */
    void exportHapdInfo();

    /**
     * Export ARICH HAPD chip info data from the database.
     */
    void exportHapdChipInfo();

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
     */
    void getBiasVoltagesForHapdChip(std::string serialNumber);

  private:

    std::vector<std::string> m_inputFilesHapdQA;
    std::vector<std::string> m_inputFilesAsicRoot;
    std::vector<std::string> m_inputFilesAsicTxt;
    std::vector<std::string> m_inputFilesHapdQE;

    ClassDef(ARICHDatabaseImporter, 0);
  };

} // Belle2 namespace
#endif
