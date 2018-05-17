/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /** This module writes data from a .dat file to a ROOT database payload. */

  class ECLEoPDatabaseImporter {

  public:

    /** Constructor */
    ECLEoPDatabaseImporter(std::string inputFileName, std::string m_name);

    /** Destructor */
    virtual ~ECLEoPDatabaseImporter() {};

    /** Import parameters of E/p PDF */
    void importPDF();

  private:

    std::string m_inputFileName; /**< Name of input .dat file */
    std::string m_name;          /**< Name of database ROOT file */

    ClassDef(ECLEoPDatabaseImporter, 2); /**< ClassDef */
  };

}
