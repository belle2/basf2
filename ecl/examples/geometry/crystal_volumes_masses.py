##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Compute the volume and mass of every ECL crystal shape from the Geant4 solids.

The solids are built exactly as in GeoECLCreator::wrapped_crystal(): the shape
parameters are read with load_shapes() (ecl/data/crystal_shape_*.dat, or the
ECLCrystalsShapeAndPosition payload) and turned into BelleCrystal solids via
shape_t::get_solid() with zero wrapping thickness, i.e. the bare CsI(Tl) crystal.

The volume is BelleCrystal::GetCubicVolume(), exact (sum over the triangulated
faces). The mass uses the density of G4_CESIUM_IODIDE (4.51 g/cm3), the material
assigned to the crystals in the simulation.

These numbers reproduce the Volume(cc) and Weight(kg) columns of
ecl/data/crystal_shape_*.dat (Ikeda's thesis tables): exactly for the barrel
(e.g. 987.80 and 1112.22 cc), and within ~0.1-0.3% for the endcaps (e.g. forward
shape 1: 921.7 vs 923 cc). For the per-cellID mapping see crystal_shapes_per_cellid.py.
"""

import ROOT

ROOT.gSystem.Load("libecl.so")
ROOT.gInterpreter.Declare(r"""
#include <ecl/geometry/shapes.h>
#include <ecl/geometry/BelleCrystal.h>
#include <G4SystemOfUnits.hh>

/** Volume of every crystal shape: {part, shape number, volume [cm3]} */
std::vector<std::array<double, 3>> eclShapeVolumes()
{
  using namespace Belle2::ECL;
  auto shapesAndPositions = loadCrystalsShapeAndPosition();
  std::vector<std::array<double, 3>> volumes;
  for (ECLParts part : {ECLParts::forward, ECLParts::barrel, ECLParts::backward}) {
    for (const shape_t* shape : load_shapes(&shapesAndPositions, part)) {
      G4Translate3D shift;
      const double wrapThickness = 0; // bare crystal, no wrapping
      auto* crystal = static_cast<BelleCrystal*>(shape->get_solid("crystal", wrapThickness, shift));
      volumes.push_back({double(part), double(shape->nshape), crystal->GetCubicVolume() / cm3});
    }
  }
  return volumes;
}
""")

#: Density of G4_CESIUM_IODIDE, the material of the simulated crystals [g/cm3]
CSI_DENSITY = 4.51

part_names = {0: "forward", 1: "barrel", 2: "backward"}
for part, shape_number, volume in ROOT.eclShapeVolumes():
    mass = volume * CSI_DENSITY / 1000  # kg
    print(f"{part_names[int(part)]:8s} {int(shape_number):3d}  V={volume:8.2f} cm3  m={mass:6.3f} kg")
