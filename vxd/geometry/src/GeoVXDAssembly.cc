/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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


