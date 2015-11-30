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
#include <TTimeStamp.h>

namespace Belle2 {

  //! ARICH database importer.
  /*!
    This module writes data to database.
  */
  class ARICHDatabaseImporter {

  public:
    ARICHDatabaseImporter(std::vector<std::string> inputFilesHapdQA, std::vector<std::string> inputFilesAsicRoot,
                          std::vector<std::string> inputFilesAsicTxt);

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
     * Get lists of problematic AISC channels.
     */
    std::vector<int> channelsList(std::string asicSerial, std::string type);

    /**
     * Import ARICH FEB test data in the database.
     */
    void importFebTest();

    /**
     * Convert date (FEB) to TTimeStamp.
     */
    TTimeStamp timedate2(std::string time);

    /**
     * Export ARICH FEB test data from the database.
     */
    void exportFebTest();

  private:

    std::vector<std::string> m_inputFilesHapdQA;
    std::vector<std::string> m_inputFilesAsicRoot;
    std::vector<std::string> m_inputFilesAsicTxt;

    ClassDef(ARICHDatabaseImporter, 0);
  };

} // Belle2 namespace
#endif
