# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

mypath = Path()
outputFile = sys.argv[1]

reset_database()
use_database_chain()
use_central_database('data_reprocessing_validation_release-03-00-00')

mypath.add_module('RootInput')
mypath.add_module('HistoManager', histoFileName=outputFile)
mypath.add_module('Gearbox')
mypath.add_module('Geometry')
mypath.add_module('SetupGenfitExtrapolation')

dqm = register_module('SVDDQMEfficiency')
dqm.set_log_level(LogLevel.INFO)
dqm.param("svdClustersName", "SVDClusters")
dqm.param("tracksName", "RecoTracks")
dqm.param("histogramDirectoryName", "svdeff")
dqm.param("binsU", 4)
dqm.param("binsV", 6)
dqm.param("distCut", 0.0500)
dqm.param("pCut", 0)
dqm.param("useAlignment", True)
dqm.param("maskStrips", False)
dqm.param("minSVDHits", 0)
dqm.param("minPXDHits", 0)
dqm.param("momCut", 0)
dqm.param("cutBorders", True)
dqm.param("maskedDistance", 10)
mypath.add_module(dqm)

# Process the events
mypath.add_module('Progress')
print_path(mypath)
process(mypath)

# print out the summary
print(statistics)
