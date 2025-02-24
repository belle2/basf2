import basf2  # noqa: needed to load pythonizations
import ROOT
from ROOT.Belle2 import VxdID, PXDDigit


import unittest
import numpy as np


class TestFillReadArray(unittest.TestCase):
    '''
    Class that tests the function readArray and fillArray
    '''

    def test_readArray(self):
        l_pxd = []
        sids = np.array([1, 2, 3, 4, 5], dtype=np.ushort)
        ucells = np.array([10, 20, 30, 40, 50], dtype=np.ushort)
        vcells = np.array([15, 25, 35, 45, 55], dtype=np.ushort)
        charges = np.array([10, 50, 100, 150, 200], dtype=np.ushort)

        for sid, u, v, c in zip(sids, ucells, vcells, charges):
            l_pxd.append(PXDDigit(VxdID(int(sid)), int(u), int(v), int(c)))

        store = ROOT.Belle2.PyStoreArray("PXDDigits")
        store.registerInDataStore()

        for pxd in l_pxd:
            ptr = store.appendNew()
            ptr.__assign__(pxd)

        d = store.readArray()

        self.assertTrue(np.array_equal(d["sensorID"], sids))
        self.assertTrue(np.array_equal(d["uCellID"], ucells))
        self.assertTrue(np.array_equal(d["vCellID"], vcells))
        self.assertTrue(np.array_equal(d["charge"], charges))

        arr_ptr = store.getPtr()
        arr_ptr.Clear()

    def test_fillArray(self):
        sids = np.array([1, 2, 3, 4, 5])
        ucells = np.array([10, 20, 30, 40, 50])
        vcells = np.array([15, 25, 35, 45, 55])
        charges = np.array([10, 50, 100, 150, 200])

        store = ROOT.Belle2.PyStoreArray("PXDDigits")
        store.registerInDataStore()

        store.fillArray(sensorID=sids, uCellID=ucells, vCellID=vcells, charge=charges)

        sensorIDs, uCellIDs, vCellIDs, read_charges = [], [], [], []

        for pxd in store:
            sensorIDs.append(pxd.getSensorID().getID())
            uCellIDs.append(pxd.getUCellID())
            vCellIDs.append(pxd.getVCellID())
            read_charges.append(pxd.getCharge())

        self.assertTrue(np.array_equal(sids, np.array(sensorIDs)))
        self.assertTrue(np.array_equal(ucells, np.array(uCellIDs)))
        self.assertTrue(np.array_equal(vcells, np.array(vCellIDs)))
        self.assertTrue(np.array_equal(charges, np.array(read_charges)))

        arr_ptr = store.getPtr()
        arr_ptr.Clear()


if __name__ == "__main__":
    # test everything
    unittest.main()
