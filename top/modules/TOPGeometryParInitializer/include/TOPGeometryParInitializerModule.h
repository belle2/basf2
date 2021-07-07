/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Class for initializing TOPGeometryPar.
   *
   * The singleton class TOPGeometryPar is a central store for all the run-independent
   * parameters that we need in the TOP software. It is used when creating G4 geometry
   * (the volumes) of TOP counter, then in the packer/unpacker to convert scrodID and
   * channel numbers to the corresponding slot and pixel ID's, then in the digitizer,
   * and finally in the reconstruction.
   *
   * By default TOPGeometryPar is initialized in GeoTOPCreator called by Geometry module,
   * either from conditions DB or using XML files. But since for the TOP the G4 geometry
   * is used only in the simulation (in module FullSim), we don't need to create it
   * when processing our own data. In these cases we can replace Geometry module with
   * this one.
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

