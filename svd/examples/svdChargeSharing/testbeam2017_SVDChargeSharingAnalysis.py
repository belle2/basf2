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
import sys

fileIN = sys.argv[1]
dirOUT = sys.argv[2]

# slightly changed method from testbeam.utils: the geometry for 2017 TB is loaded from .xml file, whereas in the current
# version of the software it's created from DB by default, thus I had to add "useDB = False" parameter to the geometry
# module.


def add_geometry(
        path,
        magnet=True,
        field_override=None,
        target=None,
        geometry_xml='testbeam/vxd/2017_geometry.xml',
        excluded_components=[],
        geometry_version=1):
    additonal_components = []

    if geometry_version == 0:
        print('WARNING: overriding the setting of parameter geometry_xml! Old value: ' + geometry_xml)
        geometry_xml = 'testbeam/vxd/2017_geometry.xml'
        print('New value: ' + geometry_xml)

    if geometry_version == 1:
        print('WARNING: overriding the setting of parameter geometry_xml! Old value: ' + geometry_xml)
        geometry_xml = 'testbeam/vxd/2017_geometry_1.xml'
        print('New value: ' + geometry_xml)

    if target is not None:
        additonal_components += [target]

    if not magnet:
        excluded_components += ['MagneticField']

    if field_override is not None:
        excluded_components += ['MagneticField']
        additonal_components += ['MagneticFieldConstant']

    # Add gearbox, additonally override field value if set
    if field_override is not None:
        path.add_module('Gearbox',
                        fileName=geometry_xml,
                        override=[("/DetectorComponent[@name='MagneticFieldConstant']//Z",
                                   str(field_override),
                                   "")])
    else:
        path.add_module('Gearbox', fileName=geometry_xml)

    # Add geometry with additional/removed components
    path.add_module('Geometry', excludedComponents=excluded_components, additionalComponents=additonal_components,
                    useDB=False)


main = b2.create_path()
main.add_module('RootInput', inputFileName=str(fileIN))

# define geometry version
magnet_off = True
# run 400, p = 4 GeV, 1T B-Field
geom = 1
# run 111, p = 5 GeV, no B-Field
if (magnet_off):
    geom = 0

add_geometry(main, magnet=not magnet_off, field_override=None, target=None, geometry_version=geom)

main.add_module('SVDChargeSharingAnalysis', outputDirName=str(dirOUT), outputRootFileName='test.root',
                useTrackInfo=True, is2017TBanalysis=True)

main.add_module('Progress')
b2.print_path(main)
b2.process(main)
print(b2.statistics)
