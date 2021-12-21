#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Doxygen should skip this script
# @cond

import basf2
from ROOT import Belle2
import numpy as np
import pandas as pd

# --------------------------------


class printCrystals(basf2.Module):
    """Module to save crystal information to dataframes"""

    def __init__(self):
        super().__init__()  # don't forget to call parent constructor
        self.obj_eclgeometrypar = Belle2.ECL.ECLGeometryPar.Instance()
        self.obj_neighbours1 = Belle2.ECL.ECLNeighbours("N", 1)
        self.obj_neighbours1C = Belle2.ECL.ECLNeighbours("NC", 1)

        self.obj_neighbours2 = Belle2.ECL.ECLNeighbours("N", 2)
        self.obj_neighbours2C = Belle2.ECL.ECLNeighbours("NC", 2)

        self.obj_neighbours1legacy = Belle2.ECL.ECLNeighbours("NLegacy", 1)
        self.obj_neighbours1Clegacy = Belle2.ECL.ECLNeighbours("NCLegacy", 1)

        self.obj_neighbours2legacy = Belle2.ECL.ECLNeighbours("NLegacy", 2)
        self.obj_neighbours2Clegacy = Belle2.ECL.ECLNeighbours("NCLegacy", 2)

        self.list_pos = []
        self.list_theta = []
        self.list_phi = []

        self.list_neighbours1 = []
        self.list_nneighbours1 = []

        self.list_neighbours1c = []
        self.list_nneighbours1c = []

        self.list_neighbours2 = []
        self.list_nneighbours2 = []

        self.list_neighbours2c = []
        self.list_nneighbours2c = []

        self.list_neighbours1legacy = []
        self.list_nneighbours1legacy = []

        self.list_neighbours1clegacy = []
        self.list_nneighbours1clegacy = []

        self.list_neighbours2legacy = []
        self.list_nneighbours2legacy = []

        self.list_neighbours2clegacy = []
        self.list_nneighbours2clegacy = []

    def initialize(self):
        for idx in range(8736):
            pos = self.obj_eclgeometrypar.getCrystalPos(idx)
            theta = pos.theta()
            phi = pos.phi()
            x = pos.x()
            y = pos.y()
            z = pos.z()

            self.obj_eclgeometrypar.Mapping(idx)
            thetaid = int(self.obj_eclgeometrypar.GetThetaID())
            phiid = int(self.obj_eclgeometrypar.GetPhiID())

            self.list_pos.append(np.array([x, y, z, phi, theta, phiid, thetaid]))

            nn1 = np.array(self.obj_neighbours1.getNeighbours(idx + 1))
            nn1c = np.array(self.obj_neighbours1C.getNeighbours(idx + 1))
            nn1legacy = np.array(self.obj_neighbours1legacy.getNeighbours(idx + 1))
            nn1clegacy = np.array(self.obj_neighbours1Clegacy.getNeighbours(idx + 1))
            self.list_neighbours1.append(nn1)
            self.list_nneighbours1.append(len(nn1))

            self.list_neighbours1c.append(nn1c)
            self.list_nneighbours1c.append(len(nn1c))

            self.list_neighbours1legacy.append(nn1legacy)
            self.list_nneighbours1legacy.append(len(nn1legacy))

            self.list_neighbours1clegacy.append(nn1clegacy)
            self.list_nneighbours1clegacy.append(len(nn1clegacy))

            nn2 = np.array(self.obj_neighbours2.getNeighbours(idx + 1))
            nn2c = np.array(self.obj_neighbours2C.getNeighbours(idx + 1))
            nn2legacy = np.array(self.obj_neighbours2legacy.getNeighbours(idx + 1))
            nn2clegacy = np.array(self.obj_neighbours2Clegacy.getNeighbours(idx + 1))

            self.list_neighbours2.append(nn2)
            self.list_nneighbours2.append(len(nn2))

            self.list_neighbours2legacy.append(nn2legacy)
            self.list_nneighbours2legacy.append(len(nn2legacy))

            self.list_neighbours2c.append(nn2c)
            self.list_nneighbours2c.append(len(nn2c))

            self.list_neighbours2clegacy.append(nn2clegacy)
            self.list_nneighbours2clegacy.append(len(nn2clegacy))

        # crystal coordinates
        dfcoord = pd.DataFrame(data=self.list_pos)
        dfcoord.to_csv('crystals_coordinates.csv', index=True, header=False)

        # neighbour maps: N1 and NC1
        dfneighbours1 = pd.DataFrame(data=self.list_neighbours1)
        dfneighbours1.to_csv('crystals_neighbours1.csv', index=True, header=False)
        dfneighbours1c = pd.DataFrame(data=self.list_neighbours1c)
        dfneighbours1c.to_csv('crystals_neighbours1c.csv', index=True, header=False)

        # number of neighbours
        dfnneighbours1 = pd.DataFrame(data=self.list_nneighbours1)
        dfnneighbours1.to_csv('crystals_nneighbours1.csv', index=True, header=False)
        dfnneighbours1c = pd.DataFrame(data=self.list_nneighbours1c)
        dfnneighbours1c.to_csv('crystals_nneighbours1.csv', index=True, header=False)

        # neighbour maps: N2 and NC2
        dfneighbours2 = pd.DataFrame(data=self.list_neighbours2)
        dfneighbours2.to_csv('crystals_neighbours2.csv', index=True, header=False)
        dfneighbours2c = pd.DataFrame(data=self.list_neighbours2c)
        dfneighbours2c.to_csv('crystals_neighbours2c.csv', index=True, header=False)

        # number of neighbours
        dfnneighbours2 = pd.DataFrame(data=self.list_nneighbours2)
        dfnneighbours2.to_csv('crystals_nneighbours2.csv', index=True, header=False)
        dfnneighbours2c = pd.DataFrame(data=self.list_nneighbours2c)
        dfnneighbours2c.to_csv('crystals_nneighbours2c.csv', index=True, header=False)

        # neighbour maps: N1 and NC1 LEGACY VERSION
        dfneighbours1legacy = pd.DataFrame(data=self.list_neighbours1legacy)
        dfneighbours1legacy.to_csv('crystals_neighbours1legacy.csv', index=True, header=False)
        dfneighbours1clegacy = pd.DataFrame(data=self.list_neighbours1clegacy)
        dfneighbours1clegacy.to_csv('crystals_neighbours1clegacy.csv', index=True, header=False)

        # number of neighbours
        dfnneighbours1legacy = pd.DataFrame(data=self.list_nneighbours1legacy)
        dfnneighbours1legacy.to_csv('crystals_nneighbours1legacy.csv', index=True, header=False)
        dfnneighbours1clegacy = pd.DataFrame(data=self.list_nneighbours1clegacy)
        dfnneighbours1clegacy.to_csv('crystals_nneighbours1clegacy.csv', index=True, header=False)

        # neighbour maps: N2 and NC2 LEGACY VERSION
        dfneighbours2legacy = pd.DataFrame(data=self.list_neighbours2legacy)
        dfneighbours2legacy.to_csv('crystals_neighbours2legacy.csv', index=True, header=False)
        dfneighbours2clegacy = pd.DataFrame(data=self.list_neighbours2clegacy)
        dfneighbours2clegacy.to_csv('crystals_neighbours2clegacy.csv', index=True, header=False)

        # number of neighbours
        dfnneighbours2legacy = pd.DataFrame(data=self.list_nneighbours2legacy)
        dfnneighbours2legacy.to_csv('crystals_nneighbours2legacy.csv', index=True, header=False)
        dfnneighbours2clegacy = pd.DataFrame(data=self.list_nneighbours2clegacy)
        dfnneighbours2clegacy.to_csv('crystals_nneighbours2clegacy.csv', index=True, header=False)

    def event(self):
        """Event loop"""


# create path
main_path = basf2.create_path()

main_path.add_module('EventInfoSetter', expList=[1003])
main_path.add_module('Gearbox')
main_path.add_module('Geometry')
printCrystals = printCrystals()
main_path.add_module(printCrystals)

basf2.process(main_path)
print(basf2.statistics)

# @endcond
