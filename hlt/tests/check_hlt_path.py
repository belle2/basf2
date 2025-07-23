##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Check if the path returned by the relevant HLT functions is changed. The test fails if:
- a new module is added to the path
- a new parameter is added to a module
- the default value of a parameter is modified
If the test fails, it's enough to reproduce the logfile and commit it.
'''

import basf2 as b2
import b2test_utils as b2tu
from pxd import add_pxd_percentframe
from softwaretrigger import constants
from softwaretrigger.processing import add_hlt_processing
from tracking import add_roi_payload_assembler


b2tu.configure_logging_for_tests()

# This checks the path used by beam_reco_monitor
b2.B2INFO('basf2 path for beam_reco_monitor:')
main_monitor = b2.Path()
add_hlt_processing(main_monitor, run_type=constants.RunTypes.beam,
                   softwaretrigger_mode=constants.SoftwareTriggerModes.monitor)
b2.print_path(path=main_monitor, defaults=True)

# This checks the path used by beam_reco_filter
b2.B2INFO('basf2 path for beam_reco_filter:')
main_filter = b2.Path()
add_hlt_processing(main_filter, run_type=constants.RunTypes.beam,
                   softwaretrigger_mode=constants.SoftwareTriggerModes.filter)
b2.print_path(path=main_filter, defaults=True)

# This checks the path used by passthrough
b2.B2INFO('basf2 path for passthrough:')
main_pass = b2.Path()
add_pxd_percentframe(main_pass, fraction=0.1, random_position=True)
add_roi_payload_assembler(main_pass, ignore_hlt_decision=True)
b2.print_path(path=main_pass)
