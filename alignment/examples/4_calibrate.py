#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

set_debug_level(1000)

reset_database()
use_local_database()

# main = create_path()
# main.add_module('RootInput')
# process(main)

# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
inmod = register_module('RootInput')
inmod.param('inputFileName', 'RootOutput.root')
inmod.initialize()

# We only need to initialize Gearbox and Geometry
# in order for VXDGeoCache to know about sensors and
# gives us their list.
gear = register_module('Gearbox')
gear.initialize()
geom = register_module('Geometry')
geom.param('components', ['PXD', 'SVD'])
geom.initialize()

algo = Belle2.MillepedeAlgorithm()
algo.steering().command('method inversion 3 0.1')
algo.steering().command('entries 100')
algo.steering().command('chiscut 30. 6.')
algo.steering().command('outlierdownweighting 3')
algo.steering().command('dwfractioncut 0.1')

algo.steering().command('Parameters')
for vxdid in Belle2.VXD.GeoCache.getInstance().getListOfSensors():
    if vxdid.getLayerNumber() == 6 or (vxdid.getLayerNumber() > 3 and vxdid.getSensorNumber() == 1):
        label = Belle2.GlobalLabel(vxdid, 0)
        for ipar in range(1, 7):
            par_label = label.label() + ipar
            cmd = str(par_label) + ' 0. -1.'
            algo.steering().command(cmd)

"""
for icLayer in range(0, 57):
  cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), 1).label())  + ' 0. -1.'
  algo.steering().command(cmd)
  cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), 2).label())  + ' 0. -1.'
  algo.steering().command(cmd)
"""

algo.execute()


import ROOT


x_profile = ROOT.TH1F("x-profile", "X correction for layers", 56, 0, 56)
y_profile = ROOT.TH1F("y-profile", "Y correction for layers", 56, 0, 56)

for payload in algo.getPayloads():
    vxd = payload.object.IsA().DynamicCast(Belle2.VXDAlignment().IsA(), payload.object, False)
    cdc = payload.object.IsA().DynamicCast(Belle2.CDCCalibration().IsA(), payload.object, False)
    try:
        for icLayer in range(0, 56):
            x = cdc.get(Belle2.WireID(icLayer, 511), 0)
            y = cdc.get(Belle2.WireID(icLayer, 511), 1)
            x_profile.SetBinContent(icLayer, x)
            x_profile.SetBinError(icLayer, cdc.getError(Belle2.WireID(icLayer, 511), 0))
            y_profile.SetBinContent(icLayer, y)
            y_profile.SetBinError(icLayer, cdc.getError(Belle2.WireID(icLayer, 511), 1))
    except:
        pass

# Done in algo
# algo.commit()

chi2ndf = Belle2.PyStoreObj('MillepedeCollector_chi2/ndf', 1).obj().getObject('1.1')
pval = Belle2.PyStoreObj('MillepedeCollector_pval', 1).obj().getObject('1.1')


# import interactive
# interactive.embed()
