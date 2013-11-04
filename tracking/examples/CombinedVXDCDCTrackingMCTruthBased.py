#! /usr/bin/env python
"""
Create VXD and CDC track candidates using VXDTF and Trasan, respectively.
Merge both kinds of track candidates using the MCTrackCandCombiner.
This module uses MC truth to combine VXD and CDC tracks.
"""

from basf2 import *
from optparse import OptionParser
import os

parser = OptionParser()

parser.add_option('-t', '--decaytable',
                  dest='decaytable',
                  default=None,
                  help='User decay table for EvtGenInput.')
parser.add_option('-p', '--pdlfile',
                  default=None,
                  help='User PDL file for EvtGenInput.')
parser.add_option('-f', '--fitting',
                  dest='fitting',
                  action='store_true',
                  default=False,
                  help='Switch on fitting.')
parser.add_option('-d', '--display',
                  dest='display',
                  action='store_true',
                  default=False,
                  help='Show events in the event display.')

options = parser.parse_args()[0]

print options

# number of events etc.
eventinfosetter = register_module('EventInfoSetter')
param_eventinfosetter = {'evtNumList': [1],
                    'runList': [1],
                    'expList': [1],
                    }

# evtgen
# use user specified decaytable
evtgeninput = register_module('EvtGenInput')
if options.decaytable != None:
    evtgeninput.param('userDECFile', options.decaytable)
if options.pdlfile != None:
    evtgeninput.param('pdlFile', options.pdlfile)


# show progress of processing
progress = register_module('Progress')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'BeamPipe',
                              'PXD', 'SVD', 'CDC'])

# simulation
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
g4sim.logging.log_level = LogLevel.ERROR

# pxd digitizer and clusterizer
pxddigitizer = register_module('PXDDigitizer')
pxdclusterizer = register_module('PXDClusterizer')

# svd digitizer and clusterizer
svddigitizer = register_module('SVDDigitizer')
svdclusterizer = register_module('SVDClusterizer')

# cdc digitizer
cdcdigitizer = register_module('CDCDigitizer')

# name of the GFTrackCand collection of VXDTF and Trasan
# and the name of the combined collection (output of
# MCTrackCandCombiner)
vxdgftrackcands = 'VXDGFTrackCands'
cdcgftrackcands = 'CDCGFTrackCands'
combinedgftrackcands = 'GFTrackCands'

# vxd track finder
secSetup = ['evtgenHIGH', 'evtgenSTD', 'evtgenLOW']

vxdtf = register_module('VXDTF')
param_vxdtf = {
    'GFTrackCandidatesColName': vxdgftrackcands,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'detectorType': ['VXD'],
    'sectorSetup': secSetup,
    'calcQIType': 'circleFit',
    'filterOverlappingTCs': 'hopfield',
    'cleanOverlappingSet': True,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    }
vxdtf.param(param_vxdtf)

# trasan
trasan = register_module('Trasan')
trasan.param('GFTrackCandidatesColName', cdcgftrackcands)

# mctrackcandcombiner: this modules takes a VXD GFTrackCand collection
# and a CDC GFTrackCand collection and combines VXD and CDC track candidates
# using the MC truth. Output is a GFTrackCand collection
mctrackcandcombiner = register_module('MCTrackCandCombiner')
mctrackcandcombiner.logging.log_level = LogLevel.DEBUG
param_mctrackcandcombiner = {
                        'CDCTrackCandidatesColName': cdcgftrackcands,
                        'VXDTrackCandidatesColName': vxdgftrackcands,
                        'OutputTrackCandidatesColName': combinedgftrackcands
                        }
mctrackcandcombiner.param(param_mctrackcandcombiner)

# genfitter, uses combined GFTrackCandidates for fitting
# use DAF as fitting algorithm
genfitter = register_module('GenFitter')
genfitter.param('GFTrackCandidatesColName', combinedgftrackcands)
genfitter.param('resolveWireHitAmbi', False)

display = register_module('Display')
display.param('useClusters', True)

# path
mainpath = create_path()

mainpath.add_module(eventinfosetter)
mainpath.add_module(evtgeninput)

mainpath.add_module(progress)

mainpath.add_module(gearbox)
mainpath.add_module(geometry)

mainpath.add_module(g4sim)

mainpath.add_module(pxddigitizer)
mainpath.add_module(pxdclusterizer)

mainpath.add_module(svddigitizer)
mainpath.add_module(svdclusterizer)

mainpath.add_module(cdcdigitizer)

mainpath.add_module(vxdtf)
mainpath.add_module(trasan)

mainpath.add_module(mctrackcandcombiner)

if options.fitting:
    mainpath.add_module(genfitter)

if options.display:
    mainpath.add_module(display)

process(mainpath)

print statistics
