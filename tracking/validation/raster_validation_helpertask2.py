#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Helper Task to raster through different cut values for the validation task.
"""

import sys
print(80*'-', '\n\n\n\n\n\n', 80*'-')
with open("fullVal_cut_iterator2.txt", "w") as cut_file:
    cut_file.write(sys.argv[1])
