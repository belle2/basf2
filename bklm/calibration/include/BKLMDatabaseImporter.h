/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN, VIPIN GAUR, Z. S. Stottler                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDATABASEIMPORTER_H
#define BKLMDATABASEIMPORTER_H

#include <stdio.h>
#include <TObject.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

namespace Belle2 {

//! BKLM database importer.
  /*!
   This module writes bklm data to database
  */
  class BKLMDatabaseImporter {
  public:

//  BKLMDatabaseImporter(std::vector<std::string> inputFiles, std::vector<std::string> inputFilesAsicRoot,
//                      std::vector<std::string> inputFilesAsicTxt);

    /**
    * Constructor
    */
    BKLMDatabaseImporter();

    /**
    * Destructor
    */
    virtual ~BKLMDatabaseImporter() {};

    /**
     * Import BKLM electronics mapping in the database.
     */
    void importBklmElectronicMapping();

    /**
     * Export BKLM electronics mapping from the database.
     */
    void exportBklmElectronicMapping();

    //! Import BKLM geometry parameters into the database
    void importBklmGeometryPar();

    //! Export BKLM geometry parameters from the database
    void exportBklmGeometryPar();

    //! Import BKLM simulation parameters into the database
    void importBklmSimulationPar();

    //! Export BKLM simulation parameters from the database
    void exportBklmSimulationPar();

    //! Import BKLM bad channels into the database
    //void importBklmBadChannels();
    void importBklmBadChannels(int expNoStart, int runStart, int expNoStop, int runStop, std::string fileName);

    //! Export BKLM bad channels from the database
    void exportBklmBadChannels();

    //! Import BKLM Misalignment parameters into the database
    void importBklmMisAlignment();

    //! Export BKLM Misalignment parameters from the database
    void exportBklmMisAlignment();

    //! Import BKLM Alignment parameters into the database
    void importBklmAlignment();

    //! Export BKLM Alignment parameters from the database
    void exportBklmAlignment();

    //! Import BKLM displaced-geometry parameters into the database
    void importBklmDisplacement();

    //! Export BKLM displaced-geometry parameters from the database
    void exportBklmDisplacement();

    //! Import BKLM scintillator digitization parameters in database
    void importBklmDigitizationParams();

    //! Export BKLM scintillator digitization parameters from database
    void exportBklmDigitizationParams();

    //! Import BKLM scintillator ADC parameters in database
    void importBklmADCThreshold();

    //! Export BKLM scintillator ADC parameters from database
    void exportBklmADCThreshold();

    //! Import BKLM time window cut
    void importBklmTimeWindow();

    //! Export BKLM time window cut
    void exportBklmTimeWindow();

  private:

    ClassDef(BKLMDatabaseImporter, 0); /**< ClassDef */

  };


}

#endif
