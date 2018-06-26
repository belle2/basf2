#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Test for the hierarchy in VXD

Ensure that hierarchy can compute the same global positions
of sensors as what is stored in GeoCache.
+ checks, that moving a ladder really moves a sensor in expected manner

If anyone changes the geometry construction (e.g. ladder coordinate system)
and does not correctly change hierarchy filling, this test will fail.


"""

import basf2
from basf2 import *
import ROOT
from ROOT import Belle2


class CompareTransformationsModule(basf2.Module):
    """Python module to compare transformations stored in GeoCache and computed by the hierarchy"""

    def __init__(self):
        """ constructor """
        super().__init__()
        #: Shift of a sensor
        self.test_shift_z = 10.

    def initialize(self):
        """ module initialization - after geometry, so GeoCache is ready """
        cache = Belle2.VXD.GeoCache.getInstance()

        print("Now testing hierarchy can compute nominal sensor positions...")
        # We check the trafos for EACH sensor
        for sensor in cache.getListOfSensors():
            # Nominal trafo stored by findVolumes() after geometry is created
            nominal = cache.get(sensor).getTransformation(False)
            # Trafo computed by the stored matrix chain in hierarchy (includes alignment,
            # but as we use exp=0, run=0 we should always get zero alignment. Therefore
            # we know both matrices have to be the same. Let's compare them element by element
            reco = cache.get(sensor).getTransformation(True)

            # First rotation component
            for i in range(0, 9):
                assert(abs(nominal.GetRotationMatrix()[i] - reco.GetRotationMatrix()[i]) < 1.e-14)
            # Then translations
            for i in range(0, 3):
                assert(abs(nominal.GetTranslation()[i] - reco.GetTranslation()[i]) < 1.e-14)

    def event(self):
        """ test that moving a ladder moves the sensor in the event processing """
        cache = Belle2.VXD.GeoCache.getInstance()

        original_global_sensor_z = cache.get(Belle2.VxdID("1.1.1")).pointToGlobal(ROOT.TVector3(0, 0, 0), True)[2]

        # Now move ladder... we need a copy of the current alignment
        alignment = Belle2.PyDBObj("VXDAlignment").obj().Clone()
        # Set the ladder here, not the sensor
        alignment.set(Belle2.VxdID("1.1.0").getID(), Belle2.VXDAlignment.dW, self.test_shift_z)
        # and add the object to the database store. This will run the callback
        Belle2.DBStore.Instance().addConstantOverride("VXDAlignment", alignment)

        new_global_sensor_z = cache.get(Belle2.VxdID("1.1.1")).pointToGlobal(ROOT.TVector3(0, 0, 0), True)[2]

        # expect that sensor moved with the ladder
        print("Now testing that moving a ladder moves a sensor correspondingly...")
        assert(abs(new_global_sensor_z - original_global_sensor_z - self.test_shift_z) < 1.e-14)


main = create_path()
# No params for EventInfoSetter means exp=0, run=0 --> Monte Carlo, no alignment corrections
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])
main.add_module(CompareTransformationsModule())
process(main)
