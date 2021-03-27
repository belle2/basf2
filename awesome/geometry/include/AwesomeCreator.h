/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* AWESOME headers. */
#include <awesome/simulation/AWESOMESensitiveDetector.h>

/* Belle2 headers. */
#include <geometry/CreatorBase.h>

namespace Belle2::AWESOME {

  /** The creator for the AWESOME geometry. */
  class AwesomeCreator : public geometry::CreatorBase {

  public:

    /**
     * Default constructor.
     */
    AwesomeCreator();

    /**
     * Copy constructor, deleted.
     */
    AwesomeCreator(AwesomeCreator&) = delete;

    /**
     * Assignment operator, deleted.
     */
    AwesomeCreator& operator=(AwesomeCreator&) = delete;

    /**
     * Default destructor.
     */
    virtual ~AwesomeCreator();

    /**
     * Write something useful here.
     */
    void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

  private:

    /** The AWESOME sensitive detector. */
    AWESOMESensitiveDetector* m_sensitiveDetector;

  };

}
