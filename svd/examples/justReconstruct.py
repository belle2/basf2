#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from svd import add_svd_reconstruction

numEvents = 2000

# set this string to identify the output rootfiles
outputfile = "mytest.root"

main = b2.create_path()

b2.set_random_seed(1)

main.add_module('RootInput')

main.add_module('Gearbox')
main.add_module('Geometry')

add_svd_reconstruction(main)

'''
add_tracking_reconstruction(
    main,
    components=["SVD"])
'''

# main.add_module('RootOutput', outputFileName=outputfile)

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
