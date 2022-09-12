##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test if the DensityScaler module works.
N.B.: this test can not be run in a light-release enviroment.
'''

import basf2 as b2
import b2test_utils as b2tu

b2tu.skip_test_if_light()

import generators as ge  # noqa
import simulation as si  # noqa


main = b2.Path()

main.add_module('EventInfoSetter')

main.add_module('Progress')

ge.add_evtgen_generator(main, 'mixed')

# This is the relevant piece of this test.
main.add_module('DensityScaler', scaling={'BeamPipe': 0.85, 'ECL': 1.5})

si.add_simulation(main)

b2.process(main)
