##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This test checks if we can reconstruct cDST data when the software trigger calculation is disabled,
as it is done during the cDST production stage in a calibration loop.
'''


import basf2 as b2
import generators as ge
import simulation as si
import reconstruction as re
import b2test_utils as b2tu


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        main = b2.Path()
        main.add_module('EventInfoSetter')
        main.add_module('Progress')
        ge.add_kkmc_generator(main, 'mu-mu+')
        si.add_simulation(main)
        re.add_reconstruction(main, reconstruct_cdst='rawFormat', add_trigger_calculation=False)
        re.add_cdst_output(main)
        b2.process(main, 3)
