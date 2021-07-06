/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/geometry/GeoVXDAssembly.h>
#include <G4ReflectionFactory.hh>
#include <G4LogicalVolume.hh>

namespace Belle2 {
  namespace VXD {

    void GeoVXDAssembly::place(G4LogicalVolume* mother, const G4Transform3D& transform)
    {
      G4ReflectionFactory& reflection = *G4ReflectionFactory::Instance();
      int numberOfDaughters = mother->GetNoDaughters();
      for (Placement& p : m_volumes) {
        reflection.Place(transform * p.second, p.first->GetName(), p.first,
                         mother, false, ++numberOfDaughters, false);
      }
    }

  } //VXD namespace
} //Belle2 namespace


