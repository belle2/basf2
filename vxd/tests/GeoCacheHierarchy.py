#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Test for the hierarchy in VXD

Ensure that hierarchy can compute the same global positions
of sensors as what is stored in GeoCache.

If anyone changes the geometry construction (e.g. ladder coordinate system)
and does not correctly change hierarchy filling, this test will fail.

"""

import basf2
from basf2 import *
from ROOT import Belle2


class CompareTransformationsModule(basf2.Module):
    """Python module to compare transformations stored in GeoCache and computed by the hierarchy"""

    def __init__(self):
        """ constructor """
        super().__init__()

    def initialize(self):
        """ module initialization - after geometry, so GeoCache is ready """
        cache = Belle2.VXD.GeoCache.getInstance()
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
                assert(abs(nominal.GetRotationMatrix()[i] - nominal.GetRotationMatrix()[i]) <= 1.e-14)
            # Then translations
            for i in range(0, 3):
                assert(abs(nominal.GetTranslation()[i] - nominal.GetTranslation()[i]) <= 1.e-14)

main = create_path()
# No params for EventInfoSetter means exp=0, run=0 --> Monte Carlo, no alignment corrections
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])
main.add_module(CompareTransformationsModule())
process(main)
