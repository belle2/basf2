/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *               Giacomo De Pietro                                        *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* AWESOME headers. */
#include <online_book/awesome/simulation/AWESOMESensitiveDetector.h>

/* Belle2 headers. */
#include <framework/gearbox/GearDir.h>
#include <geometry/CreatorBase.h>

/* Geant4 headers. */
#include <G4LogicalVolume.hh>

namespace Belle2::AWESOME {

  /**
   * The creator for the AWESOME detector geometry.
   */
  class AWESOMEGeometryCreator : public geometry::CreatorBase {

  public:

    /**
     * Default constructor.
     */
    AWESOMEGeometryCreator();

    /**
     * Copy constructor, deleted.
     */
    AWESOMEGeometryCreator(AWESOMEGeometryCreator&) = delete;

    /**
     * Assignment operator, deleted.
     */
    AWESOMEGeometryCreator& operator=(AWESOMEGeometryCreator&) = delete;

    /**
     * Default destructor.
     */
    virtual ~AWESOMEGeometryCreator();

    /**
     * Create the AWESOME detector geometry in the world volume.
     * @param content Gearbox path to the detector geometry.
     * @param topVolume Geant4 world volume.
     */
    void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes);

  private:

    /** The AWESOME sensitive detector. */
    AWESOMESensitiveDetector* m_sensitiveDetector;

  };

}
