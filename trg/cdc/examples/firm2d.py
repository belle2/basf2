#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
from math import pi, tan
import os
from subprocess import call

read_tsf = True
save_outout = False
kekcc = True
btrgpc09 = False

if kekcc:
    lib_source = '/home/belle2/tasheng/tsim/'
    rdi_path = '/home/belle2/tasheng/Vivado_2017.2/lib/lnx64.o'
elif btrgpc09:
    lib_source = '/home/trgadmin/tsim/'
    rdi_path = '/home/trgadmin/Xilinx/Vivado/2017.2/lib/lnx64.o'
else:
    lib_source = ''
    rdi_path = ''

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

if not read_tsf:
    # general options
    seed = 10000
    evtnum = 10
    particlegun_params = {
        'pdgCodes': [-13, 13],
        'nTracks': 1,
        'momentumGeneration': 'inversePt',
        # 'momentumParams': [0.3, 10.],
        'momentumParams': [3., 10.],
        'thetaGeneration': 'uniform',
        # 'thetaParams': [35, 145],
        'thetaParams': [80, 100],
        'phiGeneration': 'uniform',
        'phiParams': [46, 135],
        'vertexGeneration': 'uniform',
        'xVertexParams': [0, 0.0],
        'yVertexParams': [0, 0.0],
        # 'zVertexParams': [-50.0, 50.0]}
        'zVertexParams': [-10., 10.]}

    # ------------------------- #
    # create path up to trigger #
    # ------------------------- #

    # set random seed
    b2.set_random_seed(seed)
    # suppress messages and warnings during processing:
    # b2.set_log_level(b2.LogLevel.ERROR)

main = b2.create_path()

empty_path = b2.create_path()

# z position of the two ends of the first layer used by trigger
z_SL0 = [-31 - 1.5 / tan(30 / 180. * pi), 57 + 1.5 / tan(17 / 180. * pi)]
# radius of the first layer used by trigger
r_SL0 = 18.3


class Skim(b2.Module):
    """
    Reject tracks with bad combination of z0 and theta
    """

    def initialize(self):
        """
        Initialize self.mc with MCParticles StoreArray
        """
        #: MCParticles StoreArray
        self.mc = Belle2.PyStoreArray('MCParticles')

    def event(self):
        """
        Reject tracks with bad combination of z0 and theta
        """
        self.return_value(0)
        z0 = self.mc[0].getVertex().Z()
        vec = self.mc[0].getMomentum()
        # skip the event if the track didn't reach SL0
        if z_SL0[0] < z0 + r_SL0 / vec.Pt() * vec.Z() < z_SL0[1]:
            self.return_value(1)


main.add_module('Progress')

if read_tsf:
    inputName = 'tsfout.root'
    # inputName = '~/gcr/cdc/cosmic.0001.03898.HLT1.f00007.root'
    main.add_module('RootInput', inputFileName=inputName)

else:
    main.add_module('EventInfoSetter', evtNumList=evtnum)
    main.add_module('Gearbox')
    main.add_module('Geometry', components=['BeamPipe',
                                            'PXD', 'SVD', 'CDC',
                                            'MagneticFieldConstant4LimitedRCDC'])
    particlegun = b2.register_module('ParticleGun')
    particlegun.param(particlegun_params)
    main.add_module(particlegun)

    skim = Skim()

    main.add_module(skim)
    skim.if_false(empty_path)

    main.add_module('FullSim')
    main.add_module('CDCDigitizer')

# ---------------------- #
# CDC trigger and output #
# ---------------------- #

# TSF
# main.add_module('CDCTriggerTSF',
#                 InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
#                 OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"))

    firmtsf = b2.register_module('CDCTriggerTSFFirmware')
    # firmtsf.logging.log_level = b2.LogLevel.DEBUG
    # firmtsf.logging.debug_level = 30
    # firmtsf.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
    main.add_module(firmtsf)

# 2D finder
# original2d = register_module('CDCTrigger2DFinder')
# original2d.param('testFilename', 'tracks.txt')

firm2d = b2.register_module('CDCTrigger2DFinderFirmware')
firm2d.logging.log_level = b2.LogLevel.DEBUG
firm2d.logging.debug_level = 20
firm2d.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
firm2d.param('nClocks', 32)
main.add_module(firm2d)

if save_outout:
    main.add_module('RootOutput', outputFileName='tsfout.root')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
