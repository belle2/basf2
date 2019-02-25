/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Class for initializing TOPGeometryPar.
   * This class is by default initialized in GeoTOPCreator, when Geant geometry is created.
   * Useful when Geant geometry is not needed to be created.
   */
  class TOPGeometryParInitializerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPGeometryParInitializerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

  private:

    bool m_useDB; /**< If true load the Geometry from the database instead of gearbox */

  };

} // Belle2 namespace

