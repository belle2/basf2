#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
#
#  This steering file shows how to analyze the unpacked
#  BKLM raw data in order to obtain the Hitmap.
#  Note that the BKLMDigitAnalyzer module MUST go
#  after the BKLMUnpacker module!
#
#  Usually we need to process only 10k events per run.
#
#  Contact: Giacomo De Pietro (2018)
#           giacomo.depietro@roma3.infn.it
#
########################################################

from basf2 import *

# Set the input file names
inputRoot = ['/ghi/fs01/belle2/bdata/Data/Raw/e0003/r05184/sub00/physics.0003.05184.HLT2.f00000.root',
             '/ghi/fs01/belle2/bdata/Data/Raw/e0003/r05555/sub00/physics.0003.05555.HLT2.f00000.root']
inputSRoot = ['/home/belle2/giacomo/ghi/BKLM/root_output-181119-ut3trg-2kHz-1trg200ns.sroot']

# Create the main path
main = create_path()

# Add the input files and the progress bar
main.add_module('RootInput', inputFileNames=inputRoot)
# main.add_module('SeqRootInput', inputFileNames=inputSRoot)
main.add_module('Progress')

# Add the unpacker
main.add_module('BKLMUnpacker')

# Add the digit analyzer
# Note that it must go after the unpacker
# since it requires BKLMDigit and other dataobjects
# created by the BKLMUnpacker
main.add_module('BKLMDigitAnalyzer',
                outputRootName='bklmHitmap')  # Note that .root is not needed!

# Process the main path
process(main)
print(statistics)
