#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
#
#  This steering file shows how to analyze the unpacked
#  BKLM raw data in order to obtain the Hitmap.
#  Note that the BKLMDigitAnalyzer module MUST go
#  after the KLMUnpacker module!
#
#  Usually we need to process only 10k events per run.
#
#  Contact: Giacomo De Pietro (2018)
#           giacomo.depietro@roma3.infn.it
#
########################################################

import glob
import os
import basf2

# Add a Global Tag
basf2.conditions.override_globaltags()
basf2.conditions.append_globaltag('data_reprocessing_prompt_bucket7')
basf2.conditions.append_globaltag('data_reprocessing_prompt_rel4_patch')

# Create the main path
main = basf2.create_path()

# Add the input files and the progress bar
main.add_module('SeqRootInput')
main.add_module('Progress')

# Add the unpacker
main.add_module('KLMUnpacker')

# Add the digit analyzer
# Note that it must go after the unpacker
main.add_module('BKLMDigitAnalyzer',
                outputRootName='bklmHitmap')  # Note that .root is not needed!

# Process the main path
basf2.process(main)
print(basf2.statistics)

# Prodce a .pdf file with the hitmap
# Here we process only the last file produced
# by the BKLMDigitAnalyzer module
# The command to be executed is:
# root -b 'drawHitmap.cxx("./bklmHitmap_runXXX.root")'
allFiles = glob.glob('./bklmHitmap*root')  # list all the hitmap .root files
latestFile = max(allFiles, key=os.path.getctime)  # pick the latest hitmap file
bashCommand = "root -b 'drawHitmap.cxx" + '("' + latestFile + '")' + "'"
os.system(bashCommand)
