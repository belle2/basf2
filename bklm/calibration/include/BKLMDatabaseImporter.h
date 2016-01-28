/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDATABASEIMPORTER_H
#define BKLMDATABASEIMPORTER_H

#include <TObject.h>

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

  private:

    ClassDef(BKLMDatabaseImporter, 0); /**< ClassDef */

  };


}

#endif
