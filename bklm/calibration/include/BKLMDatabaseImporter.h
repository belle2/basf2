/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN, VIPIN GAUR, Z. S. Stottler                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// ROOT include
#include <TFile.h>
#include <TTree.h>
#include <TObject.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>
#include <framework/database/DBImportArray.h>

namespace Belle2 {

//! BKLM database importer.

  /*!
   This module writes BKLM data to database
  */
  class BKLMDatabaseImporter {
  public:

    /**
    * Constructor
    */
    BKLMDatabaseImporter();

    /**
    * Destructor
    */
    virtual ~BKLMDatabaseImporter() {};

    /**
     * Load default electronics mapping.
     */
    void loadDefaultBklmElectronicMapping();

    /**
     * Set non-default lane.
     * @param[in] forward Forward.
     * @param[in] sector  Sector.
     * @param[in] leyar   Layer.
     * @param[in] lane    Lane.
     */
    void setElectronicMappingLane(
      int forward, int sector, int layer, int lane);

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
    void importBklmADCThreshold(BKLMADCThreshold* threshold);

    //! Export BKLM scintillator ADC parameters from database
    void exportBklmADCThreshold();

    //! Import BKLM time window cut
    void importBklmTimeWindow();

    //! Export BKLM time window cut
    void exportBklmTimeWindow();

    //! Import BKLM strip efficiencies into the database
    void importBklmStripEfficiency(int expStart, int runStart, int expStop, int runStop, std::string fileName);

    //! Export BKLM bad channels from the database
    void exportBklmStripEfficiency();

  private:

    /** Electronics mapping. */
    DBImportArray<BKLMElectronicMapping> m_bklmMapping;

    ClassDef(BKLMDatabaseImporter, 0); /**< ClassDef */
  };
}

