/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  //! dE/dx database importer.
  /*!
    This module writes data from e.g. a ROOT histogram to database.
  */
  class DedxDatabaseImporter {

  public:

    /**
     * Constructor
     */
    DedxDatabaseImporter(std::string inputFileName, std::string m_name);

    /**
     * Destructor
     */
    virtual ~DedxDatabaseImporter() {};

    /**
     * Import CDC wire gain calibration constants to the database
     */
    void importWireGainCalibration();

    /**
     * Import CDC run gain calibration constants to the database
     */
    void importRunGainCalibration();

    /**
     * Import CDC electron saturation calibration constants to the database
     */
    void importCosineCalibration();

  private:

    std::vector<std::string> m_inputFileNames; /**< Name of input ROOT files */
    std::string m_name; /**< Name of database ROOT file */

    ClassDef(DedxDatabaseImporter, 1); /**< ClassDef */
  };

} // Belle2 namespace
