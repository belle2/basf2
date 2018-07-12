#!/bin/zsh
# -*- coding: utf-8 -*-
# Jo-Frederik.krohn@desy.de
from modularAnalysis import *


from simulation import add_simulation
# from reconstruction import add_mdst_output
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
from generators import add_evtgen_generator
# from L1trigger import add_tsim


from basf2 import *  # or just import Module if you don't want the basf2 logging functions
from ROOT import Belle2
# import pandas as pd
import sys
import time
import glob


base_path = "/home/belle2/jkrohn/"

# convert input params
try:
    outPath = str(sys.argv[1])
except BaseException:
    outPath = base_path + '/root_files/test/KLID_MDST_TEST.root'
try:
    noEvents = int(sys.argv[2])
except BaseException:
    noEvents = 10
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
    KLMexpertPath = string(sys.argv[6])
except BaseException:
    KLMexpertPath = False


if '.root' not in outPath:
    outPath = outPath + str(noEvents) + '.root'


dec_path_string = base_path + '/dec_files/generic_Btag.dec'

path = analysis_main

setupEventInfo(noEvents, path)
add_evtgen_generator(path, 'signal', Belle2.FileSystem.findFile(dec_path_string))

add_simulation(path, bkgfiles=glob.glob('/sw/belle2/bkg/*.root'))

add_reconstruction(path)


# for m in path.modules():
#  if m.name() == "KLMExpert":
#    m.logging.log_level = LogLevel.ERROR
#    #m.logging.debug_level = 200
#    if KLMexpertPath:
#      m.logging.info("Setting KLMclassifier to {}".format(KLMexpertPath))
#      m.param("classifierPath",KLMexpertPath)

if((not useKLM)and(not useECL)):
    sys.exit("nether KLM nor ECL data will be written. Aborting...")

data_writer = register_module('DataWriter')
data_writer.param("outPath", outPath)
data_writer.param("useKLM", useKLM)
data_writer.param("useECL", useECL)
path.add_module(data_writer)


process(path)
print(statistics)
