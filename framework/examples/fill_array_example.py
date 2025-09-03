#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# Python interface to fill PyStoreArray with digits                      #
# In this example we will consider PXDDigits                             #
##########################################################################

import basf2  # noqa: needed to load pythonizations
from ROOT.Belle2 import PyStoreArray, DataStore
import numpy as np


def create_digits():
    # number of events and digits for each event
    n_events = 10
    ndigits = 10

    digits = []
    for _ in range(n_events):
        # generate some digits for the PXD sensor
        sensor_ids = np.linspace(0, 40, ndigits, dtype=int)
        ucell_ids = np.linspace(0, 250, ndigits, dtype=int)
        vcell_ids = np.linspace(0, 756, ndigits, dtype=int)
        charges = np.linspace(0, 255, ndigits, dtype=int)
        digits.append({"uCellID": ucell_ids, "vCellID": vcell_ids, "charge": charges, "sensorID": sensor_ids})

    return digits


class InjectPXDDigits(basf2.Module):
    """
    Simple basf2 module to fill a PyStoreArray with PXD digits.
    Takes as arguments the digits to fill the PyStoreArray with,
    and then for each event it calls the fillArray function.
    """

    def __init__(self, digits_arrays):
        '''
        Initialization
        --------------
        digits_array: array of digits with lenght equal to the number of events
        '''
        #: Array of digits
        self.digits_arrays = digits_arrays
        super().__init__()

    def initialize(self):
        '''
        Initialization of the PyStoreArray and registration in the DataStore
        '''
        #: PyStoreArray
        self.pxddigits = PyStoreArray("PXDDigits", DataStore.c_Event)
        self.pxddigits.registerInDataStore("PXDDigits")
        #: Iterator of the digits array
        self.it_digits = iter(self.digits_arrays)

    def event(self):
        '''
        For each event take the corresponding entry of digits_array and write it
        into the PyStoreArray
        '''
        digits = next(self.it_digits)
        keys = ["sensorID", "uCellID", "vCellID", "charge"]
        sensorID, uCellID, vCellID, charge = [
            # it's important we ensure the correct type here (unsigned short)
            digits[key].astype(np.ushort) for key in keys]

        # call the function fillArray to fill the PyStoreArray
        # Note: this function appends the new digits to the array if it is non empty
        self.pxddigits.fillArray(uCellID=uCellID, vCellID=vCellID, sensorID=sensorID, charge=charge)


digits = create_digits()
path = basf2.create_path()
path.add_module("EventInfoSetter", evtNumList=len(digits))
path.add_module("Geometry")
path.add_module(InjectPXDDigits(digits))
path.add_module("RootOutput", branchNames=["PXDDigits"], outputFileName="digits_test.root")
path.add_module("Progress")

basf2.process(path)
basf2.statistics
