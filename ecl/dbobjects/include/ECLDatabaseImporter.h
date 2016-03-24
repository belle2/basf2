/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLDATABASEIMPORTER_H
#define ECLDATABASEIMPORTER_H

#include <TObject.h>

namespace Belle2 {

  //! ECL database importer.
  /*!
    This module writes data from e.g. a ROOT histogram to database.
  */
  class ECLDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    ECLDatabaseImporter(std::vector<std::string> inputFileNames, std::string m_name);

    /**
     * Destructor.
     */
    virtual ~ECLDatabaseImporter() {};

    /**
     * Import ECL energy calibration constants to the database
     */
    void importDigitEnergyCalibration();

    /**
     * Import ECL time calibration constants to the database
     */
    void importDigitTimeCalibration();

  private:

    std::vector<std::string> m_inputFileNames; /**< Input file name */
    std::string m_name;  /**< Database object (output) file name */

    /**
     * 2: Added time calibration.
     * 1: Initial version.
     */
    ClassDef(ECLDatabaseImporter, 2);
  };

} // Belle2 namespace
#endif
