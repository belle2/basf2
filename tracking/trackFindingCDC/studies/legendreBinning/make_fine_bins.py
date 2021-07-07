#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from ROOT import Belle2  # make Belle2 namespace available # noqa
from ROOT.Belle2 import TrackFindingCDC as TFCDC


def is_power_of_two(x):
    while x % 2 == 0:
        print(x)
        x = x / 2
        if x == 1:
            return True
    else:
        return False


assert(is_power_of_two(2048))


def main():
    new_bin_bounds = []

    lowest_curv_bound = -0.02
    uppest_curv_bound = 0.14
    width = 0.00007

    lower_bound = -width / 2
    upper_bound = width / 2

    new_bin_bounds = [(lower_bound, upper_bound)]

    while upper_bound < uppest_curv_bound or not is_power_of_two(len(new_bin_bounds)):
        if width == 0.00007:
            overlap = 0
        else:
            # IMPR Adjustment to the actually density in the curvature would be great
            overlap = 3. / 5.

        lower_bound = lower_bound + (1.0 - overlap) * width

        lower_width = TFCDC.PrecisionUtil.getOriginCurvPrecision(lower_bound)
        upper_width = TFCDC.PrecisionUtil.getOriginCurvPrecision(lower_bound + lower_width)
        width = (lower_width + upper_width) / 2
        width = width if width > 0.00007 else 0.00007
        upper_bound = lower_bound + width
        new_bin_bounds.append((lower_bound, upper_bound))

        # Symmetric up to the curling curvature
        if not -lower_bound < lowest_curv_bound:
            new_bin_bounds.append((-upper_bound, -lower_bound))

    new_bin_bounds = sorted(new_bin_bounds)

    with open('new_fine_curv_bounds.txt', 'w') as curv_bounds_file:
        for bin_bound in new_bin_bounds:
            curv_bounds_file.write(str(bin_bound[0]))
            curv_bounds_file.write('\n')
            curv_bounds_file.write(str(bin_bound[1]))
            curv_bounds_file.write('\n')


if __name__ == '__main__':
    main()
