#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import glob
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output

b2.set_log_level(b2.LogLevel.ERROR)

main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')

eventinfosetter.param({'evtNumList': [10],
                       'runList': [1],
                       'expList': [0]})

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

eclpuredigi = b2.register_module('ECLDigitizerPureCsI')
eclpuredigi.param('Calibration', 1)
eclpuredigi.param('elecNoise', elenoise)
eclpuredigi.param('photostatresolution', photostat)
eclpuredigi.param('sigmaTrigger', 0)
eclpuredigi.param('LastRing', 12)

main.add_module(eclpuredigi)

EclCovMatrixNtuple = b2.register_module('EclCovMatrixNtuple')
EclCovMatrixNtuple.param('dspArrayName', 'ECLDspsPureCsI')
EclCovMatrixNtuple.param('digiArrayName', 'ECLDigitsPureCsI')
EclCovMatrixNtuple.param('outputFileName', outfile)
main.add_module(EclCovMatrixNtuple)

add_mdst_output(main, additionalBranches=['ECLDspsPureCsI'])
progress = register_module('Progress')
main.add_module(progress)

b2.process(main)
print(b2.statistics)
