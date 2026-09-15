#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import argparse
import glob

from basf2 import (
    LogLevel,
    conditions,
    create_path,
    process,
    register_module,
    set_log_level,
    set_random_seed,
    statistics,
)
from reconstruction import add_reconstruction
from simulation import add_simulation

parser = argparse.ArgumentParser()
parser.add_argument(
    '-nevt',
    default=5000,
    type=int,
    help='number of events to be generated',
)
parser.add_argument(
    '-rel',
    default='release-08-02-03',
    help='sourced release',
)
parser.add_argument(
    '-bg',
    default='0',
    help=(
        'bg scenarios, according to UWG CDR: 0, S1, S2, S3, '
        'Exp26r1485, Exp26r1968, Exp30r3493, Exp33r0274, MCbgo'
    ),
)
parser.add_argument(
    '-enrg',
    default=0.1,
    type=float,
    help='generated photon energy',
)
parser.add_argument(
    '--NoUpgrade',
    default=False,
    action='store_true',
    help='Use modules and geometry for ECL upgrade (True/False)',
)
parser.add_argument(
    '--NoElectronicNoise',
    default=False,
    action='store_true',
    help=(
        'Disable electronic noise in the ECL digitizer when '
        'simulating the waveform digitization (True/False)'
    ),
)
parser.add_argument(
    '-bgoPath',
    default='BGforOverlay.root',
    type=str,
    help=(
        'Path to the simulated bgOverlay file to be loaded '
        'when option -bg MCbgo is chosen'
    ),
)
args = parser.parse_args()

print(args.nevt, ' , ', args.bg, ' , ', args.enrg)
if args.NoElectronicNoise:
    print('Electronic noise in the ECL digitizer will be disabled.')
    elec_noise_str = '_NoElecNoise'
else:
    elec_noise_str = ''

if not args.NoUpgrade:
    fname = (
        f'./ParticleGunOutput_photon_{args.enrg}GeV_bg'
        f'{args.bg}_Up{elec_noise_str}_WF.root'
    )
else:
    fname = (
        f'./ParticleGunOutput_photon_{args.enrg}GeV_bg'
        f'{args.bg}{elec_noise_str}_WF.root'
    )

# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')

# Setting the random seed for particle generation
set_random_seed(123)

# Setting particlegun options
# see
# https://gitlab.desy.de/belle2/software/basf2/-/blob/
# main/generators/examples/ParticleGunFull.py?ref_type=heads
particlegun.param('pdgCodes', [22])
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [args.enrg])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param(
    'thetaParams',
    [17, 150],
)  # CDC acceptance, ignore first 2/last 2 rings of FWD/BWD
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])


# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Save output of simulation
output = register_module('RootOutput')

# Setting the option for all non particle gun modules
if args.bg == 'Exp26r1485':
    conditions.append_globaltag("mcrd_proc16")
    conditions.append_globaltag("data_proc16")
    conditions.append_globaltag("online")
    eventinfosetter.param(
        {
            'evtNumList': [args.nevt],
            'runList': [1485],
            'expList': [26],
        }
    )
elif args.bg == 'Exp26r1968':
    conditions.append_globaltag("mcrd_proc16")
    conditions.append_globaltag("data_proc16")
    conditions.append_globaltag("online")
    eventinfosetter.param(
        {
            'evtNumList': [args.nevt],
            'runList': [1968],
            'expList': [26],
        }
    )
elif args.bg == 'Exp30r3493':
    conditions.append_globaltag("mcrd_prompt_rel08")
    conditions.append_globaltag("data_prompt_rel08")
    conditions.append_globaltag("online")
    eventinfosetter.param(
        {
            'evtNumList': [args.nevt],
            'runList': [3493],
            'expList': [30],
        }
    )
elif args.bg == 'Exp33r0274':
    conditions.append_globaltag("mcrd_prompt_rel08")
    conditions.append_globaltag("data_prompt_rel08")
    conditions.append_globaltag("online")
    eventinfosetter.param(
        {
            'evtNumList': [args.nevt],
            'runList': [274],
            'expList': [33],
        }
    )
else:
    eventinfosetter.param(
        {
            'evtNumList': [args.nevt],
            'runList': [1],
            'expList': [0],
        }
    )

# print ('expList = ', eventinfosetter.expList)

# Set output filename
output.param('outputFileName', fname)


main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)

DATA_PROD_DIR = '/group/belle2/dataprod/BGOverlay/'

print('bg scenario is number ', args.bg)
# add machine bg
if args.bg == '0':
    add_simulation(
        main,
        simulateECLUpgrade=not args.NoUpgrade,
        ECLElectronicNoise=not args.NoElectronicNoise,
    )
else:
    if args.bg == 'S1':
        bkg_file = (
            DATA_PROD_DIR
            + 'nominal_phase3/prerelease-08-00-00a/'
            + 'overlay_scenario1/BGx1/set0/*root'
        )
    elif args.bg == 'S2':
        bkg_file = (
            DATA_PROD_DIR
            + 'nominal_phase3/prerelease-08-00-00a/'
            + 'overlay/BGx1/set0/*root'
        )
    elif args.bg == 'S3':
        bkg_file = (
            DATA_PROD_DIR
            + 'nominal_phase3/prerelease-08-00-00a/'
            + 'overlay_scenario3/BGx1/set0/*root'
        )
    elif args.bg == 'MCbgo':
        bkg_file = (
            args.bgoPath
        )
    elif args.bg == 'Exp26r1485':
        bkg_file = (
            DATA_PROD_DIR
            + 'rel8/BGOExp26rel8/release-08-00-04/'
            + 'e0026/4S/r01485/beambg/sub00/*root'
        )
    elif args.bg == 'Exp26r1968':
        bkg_file = (
            DATA_PROD_DIR
            + 'BGOrd/rel8/BGOExp26rel8/release-08-00-04/'
            + 'e0026/4S/r01968/beambg/sub00/*root'
        )
    elif args.bg == 'Exp30r3493':
        bkg_file = (
            DATA_PROD_DIR
            + 'BGOrd/rel8/BGOExp30rel8/release-08-02-05/'
            + 'e0030/4S/r03493/beambg/sub00/*root'
        )
    elif args.bg == 'Exp33r0274':
        bkg_file = (
            DATA_PROD_DIR
            + 'BGOverlay/BGOrd/rel8/BGOExp33rel8/bucket40/'
            + 'release-08-01-08/e0033/4S/r00274/beambg/sub00/*root'
        )
    print(bkg_file)
    bkg = glob.glob(bkg_file)
    add_simulation(
        main,
        simulateECLUpgrade=not args.NoUpgrade,
        ECLElectronicNoise=not args.NoElectronicNoise,
        bkgfiles=bkg,
    )

add_reconstruction(main, simulateECLUpgrade=not args.NoUpgrade)
main.add_module(output)
# add_mdst_output(main)

ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('doDigits', 1)
ecldataanalysis.param('writeToRoot', 1)
ecldataanalysis.param('rootFileName', fname)
# ecldataanalysis.param('doTracking', 1)
main.add_module(ecldataanalysis)

# Process events
process(main)

# Print call statistics
print(statistics)
