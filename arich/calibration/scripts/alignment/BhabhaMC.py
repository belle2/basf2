#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import os
from simulation import add_simulation
from optparse import OptionParser
from L1trigger import add_trigger_simulation
from reconstruction import add_cdst_output, add_reconstruction
import glob

background_files = glob.glob('/group/belle2/BGFile/OfficialBKG/phase2/pseudorandom/BGOverlay.physics.0003.*.root')
# --------------------------------------------------------------------
# Example of using ARICH reconstruction
# needs reconstructed tracks (Tracks), extrapolated to ARICH (ExtHits)
# --------------------------------------------------------------------
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=200000,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='Bhabha_Prod5_MC.root')
parser.add_option('-o', '--output', dest='cdstname',
                  default='Bhabha_Prod5_MC_cdst.root')
(options, args) = parser.parse_args()

home = os.environ['BELLE2_LOCAL_DIR']

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# reset_database()
b2.use_database_chain()
b2.use_central_database("data_reprocessing_prod5", b2.LogLevel.WARNING)

# Create path
main = b2.create_path()
empty = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [int(options.nevents)], 'runList': [1]})
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox', fileName="geometry/Beast2_phase2.xml")
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
# geometry.param('components', [
#    'MagneticField',
#    'BeamPipe',
#    'PXD',
#    'SVD',
#    'CDC',
#    'ARICH'])
main.add_module(geometry)

# to run the framework the used modules need to be registered
generator = b2.register_module('BabayagaNLOInput')
generator.param('FinalState', 'ee')
generator.param('MaxAcollinearity', 180.0)
generator.param('ScatteringAngleRange', [15., 165.])
generator.param('FMax', 75000)
generator.param('MinEnergy', 0.01)
generator.param('Order', 'exp')
generator.param('DebugEnergySpread', 0.01)
generator.param('Epsilon', 0.00005)
generatorpreselection = b2.register_module('GeneratorPreselection')
generatorpreselection.param('nChargedMin', 1)
generatorpreselection.param('nChargedMax', 999)
generatorpreselection.param('MinChargedP', 4.)
# generatorpreselection.param('MinChargedTheta', 20.)
generatorpreselection.param('MinChargedTheta', 17.)
generatorpreselection.param('MaxChargedTheta', 36.)
generatorpreselection.param('applyInCMS', False)
main.add_module(generator)
main.add_module(generatorpreselection)
generatorpreselection.if_value('!=11', empty)

# components = ['PXD', 'SVD', 'CDC']


# detector simulation (still use background mixing)
# add_simulation(main, components=components, bkgfiles=background_files, bkgOverlay=True)
# add_simulation(main, components=components)

# add simulation but remove L1 trigger simulation
allowed_components = ['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM']
add_simulation(main, components=allowed_components, bkgfiles=background_files, bkgOverlay=True)

# ARICH digitization
arich_digitizer = b2.register_module('ARICHDigitizer')
arich_digitizer.param('MagneticFieldDistorsion', 1)
main.add_module(arich_digitizer)

# L1 trigger simulation
add_trigger_simulation(main, shortTracks=True, Belle2Phase="Phase2")

# tracking
# add_tracking_reconstruction(main)
add_reconstruction(main)

# convert ARICHDigits to ARICHHits
arichHits = b2.register_module('ARICHFillHits')
arichHits.param('MagFieldCorrection', 1)
main.add_module(arichHits)

# ARICH reconstruction
# calculate PID likelihoods for all tracks
arichreco = b2.register_module('ARICHReconstructor')
# store cherenkov angle information
arichreco.param('storePhotons', 1)
arichreco.param('useAlignment', 0)
main.add_module(arichreco)

# ARICH Ntuple
# create flat ntuple for performance analysis
arichNtuple = b2.register_module('ARICHNtuple')
arichNtuple.param('outputFile', options.filename)
main.add_module(arichNtuple)

# ARICH DQM
# create DQM occupancy plots
arichdqm = b2.register_module('ARICHDQM')
main.add_module(arichdqm)

add_cdst_output(main, True, '/ghi/fs01/belle2/bdata/users/sachit/' + options.cdstname)

# Uncomment to store DataStore content to root file
# output = register_module('RootOutput')
# output.param('outputFileName', 'DataStore.root')
# main.add_module(output)

# Uncomment to show event display
# display = register_module('Display')
# display.param('showARICHHits', True)
# display.param('fullGeometry', True)
# main.add_module(display)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)
# main.add_module("PrintMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)
# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)

# Make basic performance plots
# com = 'root -l ' + options.filename + ' ' + home + '/arich/utility/scripts/plotEfficiency.C'
# os.system(com)
