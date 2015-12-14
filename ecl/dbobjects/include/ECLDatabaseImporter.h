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
    ECLDatabaseImporter(std::vector<std::string> inputFileNames, std::string m_name);

    virtual ~ECLDatabaseImporter() {};

    /**
     * Import ECL single crystal calibration constants to the database
     */
    void importDigitCalibration();

  private:

    std::vector<std::string> m_inputFileNames;
    std::string m_name;

    ClassDef(ECLDatabaseImporter, 1);
  };

} // Belle2 namespace
#endif
