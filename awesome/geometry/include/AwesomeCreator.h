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
    AwesomeCreator();
    virtual ~AwesomeCreator();
    virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
  protected:
    AWESOMESensitiveDetector* m_sensitiveDetector;
  };

}
