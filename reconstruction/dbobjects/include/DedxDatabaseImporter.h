/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef DEDXDATABASEIMPORTER_H
#define DEDXDATABASEIMPORTER_H

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
    DedxDatabaseImporter(std::vector<std::string> inputFileNames, std::string m_name);

    /**
     * Destructor
     */
    virtual ~DedxDatabaseImporter() {};

    /**
     * Import CDC wire gain calibration constants to the database
     */
    void importWireGainCalibration();

  private:

    std::vector<std::string> m_inputFileNames; /**< Name of input ROOT files */
    std::string m_name; /**< Name of database ROOT file */

    ClassDef(DedxDatabaseImporter, 1); /**< ClassDef */
  };

} // Belle2 namespace
#endif
