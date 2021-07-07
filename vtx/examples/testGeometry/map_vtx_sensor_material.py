#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file creates produces a planar X/X0 map
# for particles hitting VTX sensor at perp. incidence
#
# Usage: basf2 map_vtx_sensor_material.py -- --vtx=VTX-CMOS-{5,7}layer
#
######################################################

import basf2
import ROOT
from ROOT import Belle2
import multiprocessing
import copy

import argparse
ap = argparse.ArgumentParser()
ap.add_argument("--vtx", default="VTX-CMOS-7layer", help="VTX geometry variant from xml")
args = vars(ap.parse_args())

print("INFO: Mapping materials for " + args['vtx'])


class MatScanConfig(basf2.Module):
    """Class to determine correct MaterialScan configurations to scan a single VTX sensor"""

    #: default options for the scan
    materialscan_config = {
        'spherical': False,
        'planar': True,
        'planar.plane': 'custom',
        'planar.nU': 1000,
        'planar.nV': 1000,
        'planar.splitByMaterials': True,
        'planar.ignored': ['Air', 'G4_AIR', 'VTX-ColdAir', 'VTX-Support'],
    }
    #: depth of the scan, will be symmetric around the center of the sensitive sensor
    depth = 0.1
    #: we determine the size of the sensitive area and scan size*border to also
    #  include the sensor borders
    border = 1.3

    def __init__(self, sensors):
        """Remember the list of sensors we want to make a config for"""
        super().__init__()
        self.sensors = sensors
        self.configs = {}

    def event(self):
        """Run through all the sensors, obtain their position, orientation and
        size and create a MaterialScan config from that"""
        for sensor in sensors:
            # VxdId: (layer, ladder, sensor) object
            vxdid = Belle2.VxdID(*sensor)
            # name of the sensor
            vxdid_str = "%d.%d.%d" % sensor
            # get the sensor information
            info = Belle2.VXD.GeoCache.get(vxdid)
            # and from that the origin and direction of our scan in local
            # coordinates
            local_origin = ROOT.TVector3(0, 0, -self.depth/2)
            local_u = ROOT.TVector3(1, 0, 0)
            local_v = ROOT.TVector3(0, 1, 0)
            # and transform this to global coordinates
            global_origin = info.pointToGlobal(local_origin)
            global_u = info.vectorToGlobal(local_u)
            global_v = info.vectorToGlobal(local_v)
            # get the dimensions of the sensitive area times a border fraction
            width = max(info.getBackwardWidth(), info.getForwardWidth())/2 * self.border
            length = info.getLength()/2 * self.border
            # and create the config from all this
            self.materialscan_config['Filename'] = "MatScan-{}-planar-{}.root".format(vxdid_str, args['vtx'])
            self.materialscan_config['planar.custom'] = [
                global_origin.X(), global_origin.Y(), global_origin.Z(),
                global_u.X(), global_u.Y(), global_u.Z(),
                global_v.X(), global_v.Y(), global_v.Z(),
            ]

            minU = -width
            maxU = width
            minV = -length
            maxV = length

            self.materialscan_config['planar.maxDepth'] = self.depth
            self.materialscan_config['planar.minU'] = minU
            self.materialscan_config['planar.maxU'] = maxU
            self.materialscan_config['planar.minV'] = minV
            self.materialscan_config['planar.maxV'] = maxV

            # and store the config with it's sensor id
            self.configs[sensor] = copy.deepcopy(self.materialscan_config)


def create_geometry():
    """Create a path containing the geometry we want"""
    other_parts = ['COIL', 'STR', 'ServiceGapsMaterial', 'BeamPipe', 'Cryostat', 'FarBeamLine', 'HeavyMetalShield', 'VXDService']
    main = basf2.create_path()
    main.add_module("EventInfoSetter", evtNumList=[1])
    main.add_module("Gearbox")
    main.add_module('Geometry', useDB=False, assignRegions=True,
                    excludedComponents=['PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'] + other_parts,
                    additionalComponents=[args['vtx']],
                    )
    return main


def determine_matscan_config(sensors):
    """Create the geometry and determine the MaterialScan config for all selected sensors"""
    main = create_geometry()
    config = main.add_module(MatScanConfig(sensors))
    basf2.process(main)
    # and return the sensor=>config dictionary
    return config.configs


def run_matscans(config):
    """Create the geometry and run a set of material scans for all selected sensors"""
    main = create_geometry()
    main.add_module("FullSim")
    # add all the MaterialScans at once, no need to run them separately
    for sensor, parameters in sorted(config.items()):
        main.add_module("MaterialScan", **parameters)
    basf2.process(main)


if __name__ == "__main__":
    # sensors to create materialscan for:
    sensors = [(1, 1, 1), (2, 1, 1), (3, 1, 1), (4, 1, 1), (5, 1, 1), ]
    # run first the config creation and then the MaterialScan. This is done in a
    # multiprocessing Pool because multiple process() calls does not work
    # reliably and in this way the processing is done in a forked child which
    # avoids side effects.
    with multiprocessing.Pool(maxtasksperchild=1) as pool:
        config = pool.apply(determine_matscan_config, args=(sensors,))
        pool.apply(run_matscans, args=(config,))
