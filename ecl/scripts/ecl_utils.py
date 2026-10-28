##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Python utilities for the ECL.
"""

import basf2 as b2


class ECLDumpGeometry(b2.Module):
    """Print the location and direction of the axis of every ECL crystal, once.

    The numbers come from ECL::ECLGeometryPar, i.e. from the
    ECLCrystalsShapeAndPosition payload valid for the experiment and run being
    processed, so the geometry has to be built by the Gearbox and Geometry modules
    first.

    The table is printed in the first event, because the crystal positions are
    read from a conditions payload and are therefore only known once the run is
    started.
    """

    def __init__(self):
        """Constructor: load the ECL library and look up the number of crystals."""
        super().__init__()
        self.set_name("ECLDumpGeometry")

        import ROOT
        ROOT.gSystem.Load("libecl.so")
        ROOT.gInterpreter.Declare("#include <ecl/dataobjects/ECLElementNumbers.h>")

        #: ROOT module, kept so that event() does not have to import it again
        self.root = ROOT
        #: number of ECL crystals; cellID runs from 1 to this value
        self.n_crystals = ROOT.Belle2.ECLElementNumbers.c_NCrystals
        #: print the geometry in the first event only
        self.first_event = True

    def event(self):
        """Print the table of crystal positions in the first event."""
        if not self.first_event:
            return
        self.first_event = False

        geometry = self.root.Belle2.ECL.ECLGeometryPar.Instance()
        print("\nLocation and direction of the axis of each ECL crystal")
        print("cellID    x [cm]    y [cm]    z [cm]  axisTheta axisPhi  [rad]")
        for cell_id in range(1, self.n_crystals + 1):
            position = geometry.GetCrystalPos(cell_id - 1)
            direction = geometry.GetCrystalVec(cell_id - 1)
            print(f"{cell_id:6d} {position.X():9.4f} {position.Y():9.4f} {position.Z():9.4f} "
                  f"{direction.Theta():9.6f} {direction.Phi():9.6f}")
        print("\n")
