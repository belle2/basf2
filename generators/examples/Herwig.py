#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Example: Herwig continuum generator (ccbar production) using
`add_herwig_continuum_generator` from the `herwig` module.

Usage
-----
    basf2 Herwig.py -n 1000                             # Y(4S), default conditions (exp 0, run 0)
    basf2 Herwig.py --experiment 12 --run 798 -n 1000   # per-run beam energy; global tags required

All CLI flags (--experiment, --run and -n) are automatically forwarded to add_herwig_continuum_generator().

For use without CLI flags, pass the experiment, run, and event number to
both EventInfoSetter and add_herwig_continuum_generator() explicitly.
'''

import basf2 as b2
from herwig import add_herwig_continuum_generator

main = b2.create_path()

main.add_module('EventInfoSetter',
                # expList=[exp],
                # runList=[run],
                # evtNumList=[nevents],
                )

add_herwig_continuum_generator(path=main, finalstate='ccbar',
                               # expList=[exp],
                               # runList=[run],
                               # nevents=nevents,
                               )

main.add_module('Progress')
# main.add_module('RootOutput', outputFileName='herwig_ccbar.root')
# main.add_module('PrintMCParticles', logLevel=b2.LogLevel.DEBUG, onlyPrimaries=False)

b2.process(main, calculateStatistics=True)
print(b2.statistics)
