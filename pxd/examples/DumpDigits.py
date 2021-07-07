#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

# Some ROOT tools
from ROOT import Belle2


class DumpDigits(b2.Module):

    """A simple module to dump PXD digits."""

    def __init__(self):
        """Initialize the module"""

        super(DumpDigits, self).__init__()
        #: Output file object.
        self.dumpfile = 'PXDDigitsDump.txt'
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)

    def beginRun(self):
        """ Write legend for file columns """
        with open(self.dumpfile, 'w') as dumpfile:
            dumpfile.write('vxd.id layer ladder sensor digit.u digit.v digit.charge ')

    def event(self):
        """Find clusters with a truehit and print some stats."""

        digits = Belle2.PyStoreArray('PXDDigits')
        # nDigits = digits.getEntries()
        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        with open(self.dumpfile, 'a') as dumpfile:
            s = ''
            for digit in digits:
                # Sesnor identification
                sensorID = digit.getSensorID().getID()
                [layer, ladder, sensor] = self.decode(sensorID)
                s += '{sID} {layer:3d} {ladder:3d} {sensor:3d} {u:6d} {v:6d} {c:8.1f}'.format(
                    sID=sensorID,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    u=digit.getUCellID(),
                    v=digit.getVCellID(),
                    c=digit.getCharge()
                )
                s += '\n'
            dumpfile.write(s)

    def terminate(self):
        """ Nothing."""

    def decode(self, vxdid):
        """ Utility to decode sensor IDs """

        result = []
        for f in self.vxdid_factors:
            result.append(vxdid // f)
            vxdid = vxdid % f

        return result
