#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2020  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Dmitry Matvienko, Iana Antonova (dmitry.matvienko@gmail.com, yanansk11@gmail.com)
#
# This software is provided "as is" without any warranty.
#
# eclDQManalysis creates DQM analysis histograms from raw histograms
#
# Usage: basf2 eclDQManalysis.py input_files output_file
#   input_files: one of the raw dqm files placed under /group/belle2/phase3/dqm/
#
#   output_file: dqm output file placed under /group/belle2/group/detector/ECL/tmp/DQManalysis/
# -----------------------------------------------------------------------------------------------

import basf2 as b2
from sys import argv

if len(argv) < 3:
    print()
    print(f'Usage: {argv[0]} input_filename output_filename OR {argv[0]} exp run')
    print()
    exit(1)

if argv[1].isnumeric() and argv[2].isnumeric():
    exp = int(argv[1])
    run = int(argv[2])
    input_filename = f'/group/belle2/phase3/dqm/dqmsrv1/e{exp:04}/dqmhisto/hltdqm_e{exp:04}r{run:06}.root'
    output_filename = f'/group/belle2/group/detector/ECL/tmp/DQManalysis/hltdqm_e{exp:04}r{run:06}.root'
else:
    input_filename = argv[1]
    output_filename = argv[2]

b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', input_filename)
inroot.param('SelectHistograms', "ECL/*")
main.add_module(inroot)

ecl = b2.register_module('DQMHistAnalysisECL')
main.add_module(ecl)

outroot = b2.register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases
outroot.param('HistoFile', output_filename)
main.add_module(outroot)

b2.print_path(main)
# Process all events
b2.process(main)
