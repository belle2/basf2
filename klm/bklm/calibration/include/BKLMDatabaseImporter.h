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
#include <TObject.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include <klm/bklm/dbobjects/BKLMADCThreshold.h>
#include <klm/bklm/dbobjects/BKLMElectronicsMap.h>
#include <framework/database/DBImportObjPtr.h>

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
     * @param[in] section Section.
     * @param[in] sector  Sector.
     * @param[in] leyar   Layer.
     * @param[in] lane    Lane.
     */
    void setElectronicMappingLane(
      int section, int sector, int layer, int lane);

    /**
     * Import BKLM electronics mapping in the database.
     */
    void importBklmElectronicMapping();

    //! Import BKLM geometry parameters into the database
    void importBklmGeometryPar();

    //! Import BKLM simulation parameters into the database
    void importBklmSimulationPar(int expStart, int runStart, int expStop, int runStop);

    //! Import BKLM Alignment parameters into the database
    void importBklmAlignment();

    //! Import BKLM displaced-geometry parameters into the database
    void importBklmDisplacement();

    //! Import BKLM scintillator digitization parameters in database
    void importBklmDigitizationParams();

    //! Import BKLM scintillator ADC parameters in database
    void importBklmADCThreshold(BKLMADCThreshold* threshold);

    //! Import BKLM time window cut
    void importBklmTimeWindow();

  private:

    /** Electronics mapping. */
    std::vector< std::pair<uint16_t, BKLMElectronicsChannel> > m_ElectronicsChannels;

    /** Class version. */
    ClassDef(BKLMDatabaseImporter, 1);

  };
}
