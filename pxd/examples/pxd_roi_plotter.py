#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import basf2 as b2
from ROOT import gROOT

import PXDROIUnpackerModule

# prevent Canvases from popping up on x11
gROOT.SetBatch()

main = b2.create_path()

# main.add_module('RootInput')
main.add_module('SeqRootInput')

main.add_module('Progress')

main.add_module('PXDUnpacker')

'''Unpack ROIs from ONSEN output'''
main.add_module(PXDROIUnpackerModule.PXDRawROIUnpackerModule())

'''Unpack ROIs from HLT Payload (depends if there are in the sroot file)'''
# main.add_module(PXDROIUnpackerModule.PXDPayloadROIUnpackerModule())

plotter = b2.register_module('PXDROIPlot')
plotter.param("ROIsName", "ROIs")  # calculated on HLT ... before Payload assemly
# plotter.param("ROIsName","PXDROIsPayHLT") # calculated on HLT ... unpacked from payload (depends whats in the sroot file)
plotter.param("DCROIsName", "PXDROIsDC")  # output from DATCON by ONSEN
plotter.param("HLTROIsName", "PXDROIsHLT")  # output from HLT by ONSEN
main.add_module(plotter)

b2.process(main)
