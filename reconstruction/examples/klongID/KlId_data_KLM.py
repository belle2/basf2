#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Jo-Frederik.krohn@desy.de, benjamin.oberhof@lnf.infn.it

import basf2 as b2
import modularAnalysis as ma

from simulation import add_simulation
# from reconstruction import add_mdst_output
from reconstruction import add_reconstruction
from generators import add_evtgen_generator
# import pandas as pd
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


if '.root' not in outPath:
    outPath = outPath + str(noEvents) + '.root'

# dec_path_string = base_path + '/dec_files/generic_Btag.dec'
mypath = b2.Path()

# '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007427/s00/e1003/4S/r00000/mixed/mdst/sub00/mdst_000*.root'
# my_path.add_module('RootInput', inputFileNames=inputFilename)

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
#      m.param("classifierPath",KLMexpertPath)

if((not useKLM) and (not useECL)):
    sys.exit("nether KLM nor ECL data will be written. Aborting...")

data_writer = b2.register_module('DataWriter')
data_writer.param("outPath", outPath)
data_writer.param("useKLM", useKLM)
data_writer.param("useECL", useECL)
mypath.add_module(data_writer)

b2.process(mypath)
print(b2.statistics)
