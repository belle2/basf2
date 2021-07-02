/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  //! dE/dx database importer.
  /*!
    This module writes data from e.g. a ROOT histogram to database.
  */
  class CDCDedxDatabaseImporter {

  public:

    /**
     * Constructor
     */
    CDCDedxDatabaseImporter(std::string inputFileName, const std::string& m_name);

    /**
     * Destructor
     */
    virtual ~CDCDedxDatabaseImporter() {};

    /**
     * Import a set of dedx:momentum pdfs
     */
    void importPDFs();

    /**
     * Import a scale factor to make electron dE/dx ~ 1
     */
    void importScaleFactor(double scale);

    /**
     * Import parameters for the hadron correction
     */
    void importHadronCorrection();

    /**
     * Import predicted mean parameters to the database
     */
    void importMeanParameters();

    /**
     * Import predicted resolution parameters to the database
     */
    void importSigmaParameters();

  private:

    std::vector<std::string> m_inputFileNames; /**< Name of input ROOT files */
    std::string m_name; /**< Name of database ROOT file */

  };

} // Belle2 namespace
