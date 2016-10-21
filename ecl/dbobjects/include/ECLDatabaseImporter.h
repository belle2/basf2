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
#include <TGraph.h>
#include <TFile.h>

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

    /**
     * Import ECL shower shape corrections to second moment to the database
     */
    void importShowerShapesSecondMomentCorrections();

  private:

    std::vector<std::string> m_inputFileNames; /**< Input file name */
    std::string m_name;  /**< Database object (output) file name */

    /**
     * Extract a TGraph from file with graphName. The file is assumed to be valid (pointer valid and not zombie). If graphName doesn't exist in file, do B2FATAL.
     */
    TGraph* getSecondMomentCorrectionTgraph(TFile* file, const std::string& graphName) const;

    /**
     * 1: Initial version.
     * 2: Added time calibration.
     * 3: Added Second moment importer.
     */
    ClassDef(ECLDatabaseImporter, 3);
  };

} // Belle2 namespace
#endif
