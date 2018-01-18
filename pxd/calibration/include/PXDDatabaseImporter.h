/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * PXD database importer.
   * This module writes dbojects to database or retrieves them from database.
   */
  class PXDDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    PXDDatabaseImporter()
    {}

    /**
     * Destructor
     */
    virtual ~PXDDatabaseImporter()
    {}

    /**
     * This method imports to the database the pixel
     * noise calibrations from local runs.
     * @param fileNames white space seperated list of file name
     */
    void importPXDNoiseCalibrations(std::string fileNames);

    /**
     * This method imports to the database the list of dead pixels
     * found from local runs.
     * @param fileNames white space seperated list of file name
     */
    void importPXDDeadPixelCalibrations(std::string fileNames);

    /**
     * This method imports to the database the pixel gain
     * calibrations from local runs.
     * @param fileNames white space seperated list of file name
     */
    void importPXDGainCalibrations(std::string fileNames);

    /**
     * This method imports to the database the pedestal
     * calibrations from local runs.
     * @param fileNames white space seperated list of file name
     */
    void importPXDPedestalCalibrations(std::string fileNames);

    /**
     * Generate and import a (random, fake) channel mask for testing
     * @param fractionDead the fraction of dead PMs to randomly assign
     * @param fractionHot the fraction of noisy PMs to randomly assign
     */
    void generateFakeChannelMask(double fractionDead, double fractionHot);

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
