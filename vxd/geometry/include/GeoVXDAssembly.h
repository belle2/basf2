/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GEOVXDASSEMBLY_H
#define GEOVXDASSEMBLY_H

#include <vector>
#include <G4Transform3D.hh>

class G4LogicalVolume;

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
      void place(G4LogicalVolume* mother, const G4Transform3D& transform);

      /** Add a volume to the assembly
       * @param volume Volume to be added
       * @param transform Transformation to be applied to the volume
       */
      void add(G4LogicalVolume* volume, const G4Transform3D& transform = G4Transform3D())
      {
        m_volumes.push_back(std::make_pair(volume, transform));
      }

      /** Add a assembly to the assembly
       * @param assembly assembly to be added
       * @param transform Transformation to be applied to the volume
       */
      void add(const GeoVXDAssembly& assembly, const G4Transform3D& transform = G4Transform3D())
      {
        for (const Placement& p : assembly.m_volumes) {
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
