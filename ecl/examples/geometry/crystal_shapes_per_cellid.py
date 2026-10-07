##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Map every ECL cellID (1..8736) to its crystal shape, volume and mass.

Everything is taken from the plain text tables in ecl/data, no geometry is built:

    cellID -> (thetaID, phiID)                      ECLGeometryPar mapping tables
           -> crystal placement index               as in ECLGeometryPar::read()
           -> shape number (nshape)                 crystal_placement_<part>.dat
           -> volume and mass of that shape         crystal_shape_<part>.dat

The Volume(cc) column of crystal_shape_<part>.dat is reproduced by the Geant4
BelleCrystal solids used in the simulation (see crystal_volumes_masses.py), so
these numbers are consistent with the simulated geometry.
"""

import basf2

#: Total number of ECL crystals (cellID runs from 1 to this value)
N_CRYSTALS = 8736

#: Number of polar rings (thetaID runs from 0 to this value - 1)
N_THETA_RINGS = 69

#: Number of crystal placements listed in the crystal_placement_<part>.dat files
N_PLACEMENTS = 224

# The three tables below are copied from Mapping_t in ecl/geometry/src/ECLGeometryPar.cc.

#: Per-ring correction to the first cellID of the ring: cellID(thetaID, 0) = 16 * corr + 128 * thetaID + 1
RING_START_CORRECTION = ([0, -5, -10, -14, -18, -22, -24, -26, -28, -30, -32, -34, -33]
                         + list(range(-32, 14))
                         + [14, 15, 16, 14, 12, 10, 8, 6, 2, -2])

#: Number of distinct crystal shapes in a ring; the shapes repeat with this period in phi
SHAPES_PER_RING = [3, 3, 4, 4, 4, 6, 6, 6, 6, 6, 6, 9, 9] + [2] * 46 + [9, 9, 6, 6, 6, 6, 6, 4, 4, 4]

#: Index of the first crystal of a ring in the placement list below
RING_PLACEMENT_OFFSET = ([0, 3, 6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63]
                         + list(range(132, 224, 2))
                         + [72, 81, 90, 96, 102, 108, 114, 120, 124, 128])


def data_rows(filename):
    """Yield the whitespace-separated fields of every non-empty, non-comment line."""
    for line in open(basf2.find_file(f"ecl/data/{filename}")):
        fields = line.split("#")[0].split()
        if fields:
            yield fields


def part_placements(part):
    """Return the (part, shape number) of every crystal placement of an ECL part.

    Same selection as load_placements(): the entries with nshape >= 1000 are the
    global transformations of the part, not crystals, and are skipped.
    """
    return [(part, int(fields[0])) for fields in data_rows(f"crystal_placement_{part}.dat")
            if len(fields) == 7 and int(fields[0]) < 1000]


#: (part, shape number) of every crystal placement, ordered as in ECLGeometryPar::read()
#: indices 0-71 are forward, 72-131 backward, 132-223 barrel
placements = part_placements("forward") + part_placements("backward") + part_placements("barrel")
assert len(placements) == N_PLACEMENTS

#: Volume [cc] and mass [kg] of every crystal shape, keyed by (part, shape number)
shape_volume_mass = {(part, int(fields[0])): (float(fields[-2]), float(fields[-1]))
                     for part in ("forward", "barrel", "backward")
                     for fields in data_rows(f"crystal_shape_{part}.dat")}


def crystal_info(cell_id):
    """Return (thetaID, phiID, part, shape number, volume [cc], mass [kg]) of a cellID."""
    index = cell_id - 1
    theta_id = max(i for i in range(N_THETA_RINGS) if RING_START_CORRECTION[i] * 16 + i * 128 <= index)
    phi_id = index - RING_START_CORRECTION[theta_id] * 16 - theta_id * 128
    part, shape_number = placements[RING_PLACEMENT_OFFSET[theta_id] + phi_id % SHAPES_PER_RING[theta_id]]
    volume, mass = shape_volume_mass[(part, shape_number)]
    return theta_id, phi_id, part, shape_number, volume, mass


if __name__ == "__main__":
    print(f"{'cellID':>6s} {'thetaID':>7s} {'phiID':>5s} {'part':8s} {'shape':>5s} "
          f"{'V [cc]':>8s} {'m [kg]':>6s}")
    for cell_id in range(1, N_CRYSTALS + 1):
        theta_id, phi_id, part, shape_number, volume, mass = crystal_info(cell_id)
        print(f"{cell_id:6d} {theta_id:7d} {phi_id:5d} {part:8s} {shape_number:5d} "
              f"{volume:8.2f} {mass:6.3f}")
