#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Perform script check for calibrations.
'''

from pxd.calibration import hot_pixel_mask_calibration, gain_calibration

if __name__ == '__main__':
    cal_mask = hot_pixel_mask_calibration(input_files=[])
    cal_gain = gain_calibration(input_files=[])
