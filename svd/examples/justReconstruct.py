#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from svd import add_svd_reconstruction
from tracking import add_tracking_reconstruction
import glob

numEvents = 2000

# set this string to identify the output rootfiles
outputfile = "mytest.root"

main = create_path()

set_random_seed(1)

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

print_path(main)

process(main)

print(statistics)
