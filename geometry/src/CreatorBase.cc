/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/CreatorBase.h>

namespace Belle2 {
  namespace geometry {
    void CreatorBase::createFromDB(const std::string&, G4LogicalVolume&, GeometryTypes)
    {
      //Do nothing but raise exception that we don't do anything
      throw DBNotImplemented();
    }

    // empty default implementation
    void CreatorBase::createPayloads(const GearDir&, const IntervalOfValidity&) {}
  }
} //Belle2 namespace
