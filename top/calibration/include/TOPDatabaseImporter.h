/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/geometry/TOPGeometryPar.h>
#include <TObject.h>
#include <string>

namespace Belle2 {

  /**
   * TOP database importer.
   * This module writes data to database or retrieves them from database.
   * More functions will come with future...
   */
  class TOPDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    TOPDatabaseImporter()
    {
      if (!m_topgp->isInitialized()) {
        GearDir content("/Detector/DetectorComponent[@name='TOP']/Content");
        m_topgp->Initialize(content);
      }
      if (!m_topgp->isInitialized())
        B2FATAL("Component TOP not found in Gearbox");
    }


    /**
     * Destructor
     */
    virtual ~TOPDatabaseImporter()
    {}

    /**
     * Import sample time calibration to database
     * @param fileName text file name with calibration constants
     */
    void importSampleTimeCalibration(std::string fileName);

    /**
     * Export sample time calibration from database
     */
    //    void exportSampleTimeCalibration();


  private:

    /** geometry parameters */
    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance();

    ClassDef(TOPDatabaseImporter, 1);  /**< ClassDef */
  };

} // Belle2 namespace
