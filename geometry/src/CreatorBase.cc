/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
