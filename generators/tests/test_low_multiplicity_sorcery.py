##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''Test for checking if the low_multiplicity_sorcery package works.'''

import basf2 as b2
import b2test_utils as b2tu
import low_multiplicity_sorcery as lms


def run_lms_generation(event_codes):
    '''Create and process a path where lms.add_generators is used.'''
    with b2tu.clean_working_directory():
        n_events = 10000 if event_code is lms.EventCodes_llXX else 500
        b2.set_random_seed('lms')
        main = b2.Path()
        main.add_module('EventInfoSetter')
        lms.add_generators(path=main, event_codes=event_codes)
        b2.process(main, n_events)


# Check if llXX and hhISR productions work fine.
for event_code in [lms.EventCodes_llXX, lms.EventCodes_hhISR]:
    b2.B2INFO(f'Running the test for {"llXX" if event_code is lms.EventCodes_llXX else "hhISR"}')
    return_code = b2tu.run_in_subprocess(target=run_lms_generation, event_codes=event_code)
    if return_code != 0:
        b2.B2FATAL(f'low_multiplicity_sorcery fails when generating {"llXX" if event_code is lms.EventCodes_llXX else "hhISR"}')
