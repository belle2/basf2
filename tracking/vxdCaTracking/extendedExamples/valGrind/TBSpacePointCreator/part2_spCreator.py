#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

generateSecMap = False  # <----------- hier umschalten zwischen secMapGenerierung und VXDTF!

################# start copy from here
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
numEvents = 25
initialValue = 42

momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  ## degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)
useSimpleClusterizer = False  # <----------- hier umschalten zwischen simple(schnell) und full clusterizer(realistisch)!
useEvtGen = False  # <----------- hier umschalten zwischen evtGen( realistische events) und pGun(einfache events)!
useEDeposit = True  # <----- EnergyDeposit für Hits (zum debuggen) damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen
addBG = False  #  <---- adding Background - funzt noch net
usePXD = True

# flags für die pGun
numTracks = 1
################# end Copy here

useTFDebug = True
TFDebugLevel = 2
killThreshold = 2000
hiocThreshold = 500
cutoffTune = 0.06
minState = 2
minLayer = 4
if usePXD:
    killThreshold = 500000
    hiocThreshold = 500
    minState = 4
    cutoffTune = 0.15
secSetup = ['secMapEvtGenOnR10933June2014SVDStd-moreThan500MeV_SVD',
            'secMapEvtGenOnR10933June2014SVDStd-125to500MeV_SVD',
            'secMapEvtGenOnR10933June2014SVDStd-30to125MeV_SVD']
if usePXD:
    secSetup = ['secMapEvtGenOnR10933June2014VXDStd-moreThan500MeV_PXDSVD',
                'secMapEvtGenOnR10933June2014VXDStd-125to500MeV_PXDSVD',
                'secMapEvtGenOnR10933June2014VXDStd-30to125MeV_PXDSVD']

qiType = 'circleFit'
filterOverlaps = 'hopfield'  # hopfield
collector_info = 0
TFInfoOutputPath = 'datenAngle3D/'
activateCollector = 0  # 0 is aus, 2 is fullAnalyze

clusterType = 'fullClusterizer'
if useSimpleClusterizer:
    clusterType = 'simpleClusterizer'
particleGenType = 'pGun'
if useEvtGen:
    particleGenType = 'evtGen'
eDepType = 'eDepNo'
if useEDeposit:
    eDepType = 'eDepYes'

# allows steering initial value and numEvents by sript file
if len(argv) is 1:
    print ' no arguments given, using standard values'
elif len(argv) is 2:
    seed = int(argv[1])
else:
    seed = int(argv[2])
    numEvents = int(argv[1])

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

gearbox = register_module('Gearbox')
gearbox.param('fileName',
              'testbeam/vxd/FullTelescopeVXDTB_v2_NOTAlignedAtAll.xml')

geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes the example much faster
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
    # secSetup = \
        # ['TB3GeVFullMagnetNoAlignedSource2014May22SVD-moreThan1500MeV_SVD']
    secSetup = ['testBeamMini6GeVJune08MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
  # To turn off magnetic field:
    geometry.param('components', ['TB'])
    # secSetup = ['TB4GeVNoMagnetNoAlignedSource2014May21SVD-moreThan1500MeV_SVD']
    secSetup = ['testBeamMini6GeVJune08MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

# geometry.set_log_level(LogLevel.INFO) INFO if set to true, complete list of components can be found...

# Show progress of processing
progress = register_module('Progress')

rootFileName = \
    '{events:}simulatedEventsAndSeed{seed:}with{evtType:}Using{clusterType:}-{eDep:}May2014phi{phi:}.root'.format(
    events=numEvents,
    seed=initialValue,
    evtType=particleGenType,
    clusterType=clusterType,
    eDep=eDepType,
    phi=phi,
    )

print ''
print ' entering trackingTests reading file {fileName:}'.format(fileName=rootFileName)
print ''

spCreator = register_module('TBSpacePointCreator')
spCreator.logging.log_level = LogLevel.DEBUG
spCreator.logging.debug_level = 10
spCreator.param('OnlySingleClusterSpacePoints', False)
spCreator.param('NameOfInstance', 'couplesAllowed')

rootinput = register_module('RootInput')
rootinput.param('inputFileName', rootFileName)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(rootinput)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(spCreator)

# Process events
process(main)

print statistics
