/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOVXDASSEMBLY_H
#define GEOVXDASSEMBLY_H

#include <G4LogicalVolume.hh>
#include <G4Transform3D.hh>
#include <G4ReflectionFactory.hh>
#include <boost/foreach.hpp>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {

    /** Class to group some Geant4 volumes and place them all at once with a given transformation matrix.
     *
     * This is similar to the G4VolumeAssembly but is much simpler and with less options
     */
    class GeoVXDAssembly {
    public:
      /** Placement of a logical volume consists of the Volume and ins transformation */
      typedef std::pair<G4LogicalVolume*, G4Transform3D> Placement;

      /** Place all the volumes already added to the assembly in the given mother.
       * @param mother Pointer to the volume everything should be placed in
       * @param transform Transformation to apply to all volumes when placing them
       */
      void place(G4LogicalVolume* mother, const G4Transform3D& transform) {
        G4ReflectionFactory& reflection = *G4ReflectionFactory::Instance();
        int numberOfDaughters = mother->GetNoDaughters();
        BOOST_FOREACH(Placement & p, m_volumes) {
          reflection.Place(transform * p.second, p.first->GetName(), p.first,
                           mother, false, ++numberOfDaughters, false);
        }
      }

      /** Add a volume to the assembly
       * @param volume Volume to be added
       * @param transform Transformation to be applied to the volume
       */
      void add(G4LogicalVolume* volume, const G4Transform3D& transform = G4Transform3D()) {
        m_volumes.push_back(std::make_pair(volume, transform));
      }

      /** Add a assembly to the assembly
       * @param assembly assembly to be added
       * @param transform Transformation to be applied to the volume
       */
      void add(const GeoVXDAssembly& assembly, const G4Transform3D& transform = G4Transform3D()) {
        BOOST_FOREACH(const Placement & p, assembly.m_volumes) {
          m_volumes.push_back(std::make_pair(p.first, transform * p.second));
        }
      }

    private:
      /** Array of all volumes and their placements in the assembly */
      std::vector<Placement> m_volumes;
    };
  } //VXD namespace
} //Belle2 namespace
#endif
