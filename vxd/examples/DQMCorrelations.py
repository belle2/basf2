#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file to see DQM - basic of VXD sensors
# and correlations
#
# Sample of external file, 100 particlegun or BBbar events.
#                    (default is particlegun)
#
# Usage: basf2 DQMCorrelations.py
#      ( basf2 DQMCorrelations.py -- --h )
#
# Input: external file or None (simulation)
# Output: histograms: VXD_DQM_Histograms.root or custom...
#
# This is running ExpressRecoDQM modules for PXD/SVD and VXD (PXD-SVD correlations)
#
# Contributors: Peter Kodys
#
# Repairing broken script: Benjamin Schwenker (19. May 2018)
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from ROOT import Belle2

from svd import *
from pxd import *

import argparse
parser = argparse.ArgumentParser(description="PXD+SVD+VXD DQM correlations for Belle II + TB, show all possible histos")
parser.add_argument(
    '--magnet-off',
    dest='magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='Turn off magnetic field')

parser.add_argument('--skip-pxd-svd', dest='SkipPXDSVD', action='store_const', const=True, default=False,
                    help='Add also PXD and SVD DQM, default = True')
parser.add_argument('--data-output', dest='DataOutput', action='store_const', const=True, default=False,
                    help='Save data to output file, default = False')
parser.add_argument('--correlation-granulation', dest='CorrelationGranulation', action='store', default=1.0, type=float,
                    help='Set granulation of histogram plots, default is 1 degree, min = 0.02 degree, max = 1 degree')
parser.add_argument('--cut-correlation-sig-pxd', dest='CutCorrelationSigPXD', action='store', default=0, type=int,
                    help='Cut threshold of PXD signal for accepting to correlations, default = 0 ADU')
parser.add_argument('--cut-correlation-sig-u-svd', dest='CutCorrelationSigUSVD', action='store', default=0, type=int,
                    help='Cut threshold of SVD signal for accepting to correlations in u, default = 0 ADU')
parser.add_argument('--cut-correlation-sig-v-svd', dest='CutCorrelationSigVSVD', action='store', default=0, type=int,
                    help='Cut threshold of SVD signal for accepting to correlations in v, default = 0 ADU')
parser.add_argument('--cut-correlation-time-svd', dest='CutCorrelationTimeSVD', action='store', default=70, type=int,
                    help='Cut threshold of SVD time window for accepting to correlations, default = 70 ns')

parser.add_argument('--use-digits', dest='UseDigits', action='store', default=0, type=int,
                    help='flag <0,1> for using digits only, no cluster information will be required, default = 0')
parser.add_argument('--reduce-1d-correl-histos', dest='Reduce1DCorrelHistos', action='store', default=0, type=int,
                    help='flag <0,1> for removing of 1D correlation plots from output, default = 0')
parser.add_argument('--reduce-2d-correl-histos', dest='Reduce2DCorrelHistos', action='store', default=0, type=int,
                    help='flag <0,1> for removing of 2D correlation plots from output, default = 0')
parser.add_argument('--only-23-layers-histos', dest='Only23LayersHistos', action='store', default=0, type=int,
                    help='flag <0,1> for to keep only correlation plots between layer 2 and 3 (between PXD and SVD), default = 0')
parser.add_argument('--save-other-histos', dest='SaveOtherHistos', action='store', default=1, type=int,
                    help='flag <0,1> for creation of correlation plots for non-neighboar layers, default = 0')
parser.add_argument('--unpacking', dest='unpacking', action='store_const', const=True,
                    default=False, help='Add PXD and SVD unpacking modules to the path')
parser.add_argument('--input-file', dest='input_file', action='store', default=None, type=str,
                    help='Name of input file')
parser.add_argument('--filename-histos', dest='histo_file_name', action='store',
                    default='VXD_DQM_Histograms.root', type=str,
                    help='Name of output file with histograms in dqm option is used')
parser.add_argument('--SkipDQM', dest='SkipDQM', action='store_const', const=True, default=False, help='Produce DQM plots')
parser.add_argument('--SkipDQMExpressReco', dest='SkipDQMExpressReco', action='store_const', const=True, default=False,
                    help='Skip production of ExpressReco DQM plots')


args = parser.parse_args()

print("Final setting of arguments: ")
print("                 SkipPXDSVD: ", args.SkipPXDSVD)
print("                 DataOutput: ", args.DataOutput)
print("     CorrelationGranulation: ", args.CorrelationGranulation)
print("       CutCorrelationSigPXD: ", args.CutCorrelationSigPXD)
print("      CutCorrelationSigUSVD: ", args.CutCorrelationSigUSVD)
print("      CutCorrelationSigVSVD: ", args.CutCorrelationSigVSVD)
print("      CutCorrelationTimeSVD: ", args.CutCorrelationTimeSVD)
print("                  UseDigits: ", args.UseDigits)
print("       Reduce1DCorrelHistos: ", args.Reduce1DCorrelHistos)
print("       Reduce2DCorrelHistos: ", args.Reduce2DCorrelHistos)
print("         Only23LayersHistos: ", args.Only23LayersHistos)
print("            SaveOtherHistos: ", args.SaveOtherHistos)
print("                  unpacking: ", args.unpacking)
print("                 input_file: ", args.input_file)
print("            histo_file_name: ", args.histo_file_name)
print("                    SkipDQM: ", args.SkipDQM)
print("         SkipDQMExpressReco: ", args.SkipDQMExpressReco)


param_vxddqm = {'CutCorrelationSigPXD': args.CutCorrelationSigPXD,
                'CutCorrelationSigUSVD': args.CutCorrelationSigUSVD,
                'CutCorrelationSigVSVD': args.CutCorrelationSigVSVD,
                'CutCorrelationTimeSVD': args.CutCorrelationTimeSVD,
                'UseDigits': args.UseDigits
                }

# Now let's create a path to simulate our events. We need a bit of statistics but
# that's not too bad since we only simulate single muons
main = create_path()
main.add_module("EventInfoSetter", evtNumList=[1000])
main.add_module("Gearbox")
# we only need the vxd for this
if (args.magnet_off is True):
    main.add_module("Geometry", components=['BeamPipe', 'PXD', 'SVD'])
else:
    main.add_module("Geometry", components=['MagneticFieldConstant4LimitedRSVD',
                                            'BeamPipe', 'PXD', 'SVD'])


main.add_module("EvtGenInput")
main.add_module("FullSim")
add_pxd_simulation(main)
add_svd_simulation(main)
add_pxd_reconstruction(main)
add_svd_reconstruction(main)

if (args.SkipDQM is False):
    histomanager = register_module('HistoManager', histoFileName=args.histo_file_name)
    main.add_module(histomanager)
    if (args.SkipPXDSVD is False):
        if (args.SkipDQMExpressReco is False):
            pxddqmExpReco = register_module('PXDDQMExpressReco')
            svddqmExpReco = register_module('SVDDQMExpressReco')
            main.add_module(pxddqmExpReco)
            main.add_module(svddqmExpReco)

    if (args.SkipDQMExpressReco is False):
        vxddqmExpReco = register_module('VXDDQMExpressReco')
        vxddqmExpReco.param(param_vxddqm)
        main.add_module(vxddqmExpReco)

if (args.DataOutput is True):
    main.add_module('RootOutput')


main.add_module("Progress")

process(main)
print(statistics)
