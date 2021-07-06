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
from ROOT import Belle2
import os
import socket
from subprocess import call

read_data = True
# merger_only = True
merger_only = False
save_outout = True
fast_tsf = True

# set the host-based locations
hostname = socket.gethostname()
if 'cc.kek.jp' in hostname:
    lib_source = '/home/belle2/tasheng/tsim/'
    rdi_path = '/home/belle2/tasheng/Vivado_2017.2/lib/lnx64.o'
elif 'btrgpc09' == hostname:
    lib_source = '/home/trgadmin/tsim/'
    rdi_path = '/home/trgadmin/Xilinx/Vivado/2017.2/lib/lnx64.o'
elif 'belle2' '= hostname':
    lib_source = '/home/ta/tsim/'
    rdi_path = '/home/ta/Vivado_2017.2/lib/lnx64.o'
else:
    lib_source = ''
    rdi_path = ''

if not merger_only:
    # set run time library path
    if rdi_path not in os.environ['LD_LIBRARY_PATH']:
        print('please set environment variable first! do either')
        print('export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:' + rdi_path)
        print('or')
        print('setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:' + rdi_path)
        exit(1)

    # link to 2D design snapshot
    for link in ['xsim.dir', 'innerLRLUT.mif', 'outerLRLUT.mif']:
        if link not in os.listdir(os.getcwd()):
            call(['ln', '-s', lib_source + link])

"""
generate tracks with particle gun, simulate CDC and CDC trigger, save the output.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 10000
evtnum = 10
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'inversePt',
    'momentumParams': [2., 10.],
    'thetaGeneration': 'uniform',
    'thetaParams': [35, 145],
    'phiGeneration': 'uniform',
    'phiParams': [46, 135],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0, 0.0],
    'yVertexParams': [0, 0.0],
    'zVertexParams': [-10., 10.]}

# ------------------------- #
# create path up to trigger #
# ------------------------- #
environment = Belle2.Environment.Instance()
if not environment.getNumberEventsOverride():
    environment.setNumberEventsOverride(evtnum)

# set random seed
b2.set_random_seed(seed)
# suppress messages and warnings during processing:
# b2.set_log_level(b2.LogLevel.ERROR)

main = b2.create_path()

empty_path = b2.create_path()

# The root file is on KEKCC
if read_data:
    main.add_module('RootInput', inputFileName='/home/belle2/tasheng/gcr/cdc/cosmic.0001.03898.HLT1.f00007.root')

main.add_module('Progress')

if not read_data:
    main.add_module('EventInfoSetter', evtNumList=evtnum)
    main.add_module('Gearbox')
    main.add_module('Geometry', components=['BeamPipe',
                                            'PXD', 'SVD', 'CDC',
                                            'MagneticFieldConstant4LimitedRCDC'])
    particlegun = b2.register_module('ParticleGun')
    particlegun.param(particlegun_params)
    main.add_module(particlegun)

    main.add_module('FullSim')
    main.add_module('CDCDigitizer')

# ---------------------- #
# CDC trigger and output #
# ---------------------- #

if fast_tsf:
    main.add_module('Gearbox')
    main.add_module('Geometry', components=['BeamPipe',
                                            'PXD', 'SVD', 'CDC',
                                            'MagneticFieldConstant4LimitedRCDC'])
    main.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"))

firmtsf = b2.register_module('CDCTriggerTSFFirmware')
firmtsf.param('mergerOnly', merger_only)
firmtsf.logging.log_level = b2.LogLevel.DEBUG
firmtsf.logging.debug_level = 10
firmtsf.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
main.add_module(firmtsf)

# 2D finder

# firm2d = register_module('CDCTrigger2DFinderFirmware')
# firm2d.logging.log_level = b2.LogLevel.DEBUG
# firm2d.logging.debug_level = 20
# firm2d.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
# main.add_module(firm2d)

if save_outout:
    main.add_module('RootOutput', outputFileName='tsfout.root')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
