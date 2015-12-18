#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import sys
import glob

set_log_level(LogLevel.ERROR)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

par = sys.argv
argc = len(par)
outfile = par[1]
elenoise = float(par[2])
photostat = float(par[3])
if (argc == 5):
    bgdir = par[4]
#  bg = glob.glob('/home/denardo/belle2/bkg/*.root')
    bg = glob.glob(bgdir + '/*.root')
    add_simulation(main, bkgfiles=bg, components='ECL')
else:
    add_simulation(main, components='ECL')

# add_reconstruction(main)

eclpuredigi = register_module('ECLDigitizerPureCsI')
eclpuredigi.param('Calibration', 1)
eclpuredigi.param('elecNoise', elenoise)
eclpuredigi.param('photostatresolution', photostat)
eclpuredigi.param('sigmaTrigger', 0)
eclpuredigi.param('LastRing', 12)

main.add_module(eclpuredigi)


EclCovMatrixNtuple = register_module('EclCovMatrixNtuple')
EclCovMatrixNtuple.param('dspArrayName', 'ECLDspsPureCsI')
EclCovMatrixNtuple.param('digiArrayName', 'ECLDigitsPureCsI')
EclCovMatrixNtuple.param('outputFileName', outfile)
main.add_module(EclCovMatrixNtuple)

add_mdst_output(main, additionalBranches=['ECLDspsPureCsI'])
progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
