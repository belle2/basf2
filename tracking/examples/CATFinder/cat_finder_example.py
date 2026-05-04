##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import generators as ge
import simulation as si
from tracking.path_utils import add_cdc_track_finding


b2.set_random_seed('cat')


main = b2.Path()

main.add_module('EventInfoSetter')

main.add_module('Progress')

ge.add_kkmc_generator(main, 'mu-mu+')

si.add_simulation(main)

add_cdc_track_finding(main, use_cat_finder=True)

main.add_module('StatisticsSummary').set_name('Sum_CATFinder')

b2.print_path(main)
b2.process(main, calculateStatistics=True)
