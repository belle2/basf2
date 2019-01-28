#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Local Calibration CHECK Script
usage:
> basf2 SVDLOcalCalibrationsCheck.py -- --ref REF --check CHECK
where:
"""

from basf2 import *
import ROOT

import argparse

parser = argparse.ArgumentParser(description="SVD Calibration Check against a Reference")
parser.add_argument('--ref', metavar='reference', dest='ref', type=str, nargs=1, help='Reference rootfile')
parser.add_argument('--check', metavar='check', dest='check', type=str, nargs=1, help='Calibration to check rootfile')

args = parser.parse_args()

main = create_path()

the_ref = args.ref[0]
the_check = args.check[0]

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)
main.add_module("Gearbox")
main.add_module("Geometry")

mod = register_module("SVDLocalCalibrationsCheck")
# mod.param('plotGoodAPVs', True) #default is false
mod.param('reference_root', str(the_ref))
mod.param('check_root', str(the_check))
mod.param('outputPdfName', 'SVDLocalCalibrationsCheck.pdf')
mod.param('cutN_outliers', 5)
mod.param('cutNoise_average', 0.1)
mod.param('cutNoise_outliers', 0.3)
mod.param('cutCalPeakADC_average', 0.015)  # 0.025
mod.param('cutCalPeakADC_outliers', 0.3)
mod.param('cutPedestal_average', 0.02)
mod.param('cutPedestal_outliers', 0.3)
main.add_module(mod)

# process single event
print_path(main)
process(main)
