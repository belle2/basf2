#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2016 @ DESY Simulation
# This is the default simulation scenario for VXD beam test

from basf2 import *
from ROOT import Belle2


def add_geometry(path, magnet=True):
    path.add_module('Gearbox', fileName='testbeam/vxd/FullVXDTB2016.xml')

    if magnet:
        path.add_module('Geometry')
    else:
        path.add_module('Geometry', excludedComponents=['MagneticField'])

import argparse
parser = argparse.ArgumentParser(description="Reconstruction for DESY VXD Testbeam 2016")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--gbl-collect',
    dest='gbl_collect',
    action='store_const',
    const=True,
    default=False,
    help='Use GBLfit for track fitting and collect calibration data')
parser.add_argument(
    '--alignment',
    dest='alignment',
    action='store_const',
    const=True,
    default=False,
    help='Run Millepede alignment at end of job')
args = parser.parse_args()


# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)

# Set up DB chain
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True)
if args.local_db is not None:
    use_local_database(Belle2.FileSystem.findFile(args.local_db))
use_local_database('database.txt')

main = create_path()

main.add_module('RootInput')  # , branchNames=['EventMetaData', 'RawFTSWs', 'RawSVDs'])


main.add_module('Progress')

add_geometry(main)

process(main)
print(statistics)

algo = Belle2.MillepedeAlgorithm()
algo.steering().command('method diagonalization 3 0.1')
algo.steering().command('entries 20')
algo.steering().command('hugecut 50')
# algo.steering().command('chiscut 30. 6.')
# algo.steering().command('outlierdownweighting 3')
# algo.steering().command('dwfractioncut 0.1')

sensors_for_alignment = [
    #    '1.1.2',
    #    '2.1.2',
    #    '3.1.2',
    #    '4.1.2',
    #    '5.1.3',
    #    '6.1.3',
    #    '6.1.4',
    '7.2.1',
    '7.2.2',
    '7.2.3',
    '7.3.4',
    '7.3.5',
    '7.3.6']

for i, sensor in enumerate(sensors_for_alignment):
    sensors_for_alignment[i] = Belle2.VxdID(sensor).getID()

algo.steering().command('Parameters')
for vxdid in Belle2.VXD.GeoCache.getInstance().getListOfSensors():
    if vxdid.getID() not in sensors_for_alignment:
        label = Belle2.GlobalLabel(vxdid, 0)
        for ipar in [1, 2, 3, 4, 5, 6]:
            par_label = label.label() + ipar
            cmd = str(par_label) + ' 0.0 -1.'
            algo.steering().command(cmd)
    elif vxdid.getLayerNumber() == 7 and vxdid.getSensorNumber() == 1:
        label = Belle2.GlobalLabel(vxdid, 0)
        for ipar in [1, 2, 3, 4, 5]:
            par_label = label.label() + ipar
            cmd = str(par_label) + ' 0.0 -1.'
            algo.steering().command(cmd)
    elif vxdid.getLayerNumber() == 7 and vxdid.getSensorNumber() == 6:
        label = Belle2.GlobalLabel(vxdid, 0)
        for ipar in [1, 2, 3, 4, 5, 6]:
            par_label = label.label() + ipar
            cmd = str(par_label) + ' 0.0 -1.'
            algo.steering().command(cmd)
    else:
        label = Belle2.GlobalLabel(vxdid, 0)
        for ipar in [3, 4, 5]:
            par_label = label.label() + ipar
            cmd = str(par_label) + ' 0.0 -1.'
            algo.steering().command(cmd)

algo.invertSign()
algo.execute()
algo.commit()
