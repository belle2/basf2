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
import modularAnalysis as ma

from simulation import add_simulation
from reconstruction import add_reconstruction
from generators import add_evtgen_generator
import sys
import glob

base_path = "."

# convert input params
try:
    outPath = str(sys.argv[1])
except BaseException:
    outPath = 'root_files/test/KLID_MDST_TEST.root'
try:
    noEvents = int(sys.argv[2])
except BaseException:
    noEvents = 100
try:
    bkgScale = float(sys.argv[3])
except BaseException:
    bkgScale = 1.0
try:
    useKLM = bool(sys.argv[4])
except BaseException:
    useKLM = True
try:
    useECL = bool(sys.argv[5])
except BaseException:
    useECL = False
try:
    KLMexpertPath = str(sys.argv[6])
except BaseException:
    KLMexpertPath = False


if outPath[-5:] != '.root':
    outPath = outPath + str(noEvents) + '.root'

# dec_path_string = base_path + '/dec_files/generic_Btag.dec'
mypath = b2.Path()

# my_path.add_module('RootInput', inputFileNames=b2.find_file('mdst14.root', 'validation'))

dec_file = None
final_state = 'mixed'
ma.setupEventInfo(noEvents, mypath)

add_evtgen_generator(mypath, finalstate=final_state, signaldecfile=dec_file)

add_simulation(mypath, bkgfiles=glob.glob('/sw/belle2/bkg/*.root'))

add_reconstruction(mypath)

# for m in path.modules():
#  if m.name() == "KLMExpert":
#    m.logging.log_level = LogLevel.ERROR
#    #m.logging.debug_level = 200
#    if KLMexpertPath:
#      m.logging.info("Setting KLMclassifier to {}".format(KLMexpertPath))
#      m.param("classifierPath", KLMexpertPath)

if((not useKLM) and (not useECL)):
    sys.exit("neither KLM nor ECL data will be written. Aborting...")

data_writer = b2.register_module('DataWriter')
data_writer.param("outPath", outPath)
data_writer.param("useKLM", useKLM)
data_writer.param("useECL", useECL)
mypath.add_module(data_writer)

b2.process(mypath)
print(b2.statistics)
